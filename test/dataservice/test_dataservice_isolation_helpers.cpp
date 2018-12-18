/**
 * \file test_dataservice_isolation_helpers.cpp
 *
 * Helpers for the dataservice isolation test.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <vccert/fields.h>
#include <vccert/certificate_types.h>

#include "../../src/dataservice/dataservice_internal.h"
#include "test_dataservice_isolation.h"

using namespace std;

const uint8_t dataservice_isolation_test::dir_key[32] = {
    0x7e, 0x4b, 0xb1, 0x5d, 0xb5, 0x00, 0x41, 0x95,
    0xb0, 0xed, 0x43, 0x59, 0x43, 0x20, 0x9b, 0x72,
    0x28, 0x07, 0xad, 0xbb, 0x87, 0x70, 0x49, 0x8a,
    0xac, 0x89, 0x44, 0xcb, 0x23, 0x56, 0x67, 0x3f
};

const uint8_t dataservice_isolation_test::dummy_artifact_type[16] = {
    0xaf, 0xe8, 0x9d, 0xa0, 0xd0, 0xb8, 0x4d, 0x97,
    0x89, 0xb3, 0xd0, 0x0f, 0xa0, 0x11, 0x8a, 0x3f
};

const uint8_t dataservice_isolation_test::dummy_transaction_type[16] = {
    0x17, 0x8b, 0xb2, 0xe8, 0xa2, 0x3b, 0x4f, 0x62,
    0x88, 0xde, 0x9e, 0xbb, 0xcf, 0x75, 0xbc, 0xd2
};

const uint8_t dataservice_isolation_test::zero_uuid[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/**
 * \brief Dispose a test context.
 */
void dataservice_isolation_test::test_context_dispose(void* disp)
{
    test_context* ctx = (test_context*)disp;

    if (nullptr != ctx->config)
        dispose((disposable_t*)ctx->config);
}

/**
 * \brief Initialize a test_context structure.
 */
void dataservice_isolation_test::test_context_init(test_context* ctx)
{
    ctx->hdr.dispose = &test_context_dispose;
    ctx->config = nullptr;
}

/**
 * \brief Simple error setting override.
 */
void dataservice_isolation_test::set_error(
    config_context_t* context, const char* msg)
{
    test_context* ctx = (test_context*)context->user_context;

    ctx->errors.push_back(msg);
}

/**
 * \brief Simple value setting override.
 */
void dataservice_isolation_test::config_callback(
    config_context_t* context, agent_config_t* config)
{
    test_context* ctx = (test_context*)context->user_context;

    ctx->config = config;
}

void dataservice_isolation_test::SetUp()
{
    /* register vccrypt stuff. */
    vccrypt_suite_register_velo_v1();
    vccrypt_block_register_AES_256_2X_CBC();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create suite. */
    suite_init_result =
        vccrypt_suite_options_init(
            &crypto_suite, &alloc_opts,
            VCCRYPT_SUITE_VELO_V1);

    /* create the builder. */
    builder_opts_init_result =
        vccert_builder_options_init(
            &builder_opts, &alloc_opts, &crypto_suite);

    /* create bootstrap config. */
    bootstrap_config_init(&bconf);

    /* set up the parser context. */
    test_context_init(&user_context);
    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;
    yylex_init(&scanner);
    state = yy_scan_string("", scanner);
    yyparse(scanner, &context);

    /* set the path for running agentd. */
    getcwd(wd, sizeof(wd));
    oldpath = getenv("PATH");
    if (NULL != oldpath)
    {
        path =
            strcatv(wd, "/build/host/release/bin", ":", oldpath, NULL);
    }
    else
    {
        path = strcatv(wd, "/build/host/release/bin");
    }

    setenv("PATH", path, 1);

    logsock = dup(STDERR_FILENO);

    /* spawn the dataservice process. */
    dataservice_proc_status =
        dataservice_proc(
            &bconf, user_context.config, logsock, &datasock, &datapid,
            false);

    /* by default, we run in blocking mode. */
    nonblockdatasock_configured = false;

    /* set up directory test helper. */
    string dbpath(wd);
    dbpath += "/build/test/isolation/databases/";
    directory_test_helper::SetUp(dir_key, dbpath.c_str());
}

void dataservice_isolation_test::TearDown()
{
    directory_test_helper::TearDown();

    if (builder_opts_init_result == 0)
    {
        dispose((disposable_t*)&builder_opts);
    }

    if (suite_init_result == 0)
    {
        dispose((disposable_t*)&crypto_suite);
    }

    dispose((disposable_t*)&alloc_opts);

    /* terminate the dataservice process. */
    if (0 == dataservice_proc_status)
    {
        int status = 0;
        kill(datapid, SIGTERM);
        waitpid(datapid, &status, 0);
    }

    /* set the old path. */
    setenv("PATH", oldpath, 1);

    /* clean up. */
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
    close(logsock);
    dispose((disposable_t*)&bconf);
    dispose((disposable_t*)&user_context);
    free(path);
}

void dataservice_isolation_test::nonblockmode(
    function<void()> onRead, function<void()> onWrite)
{
    /* set the read/write callbacks. */
    this->onRead = onRead;
    this->onWrite = onWrite;

    /* handle a non-blocking event loop. */
    if (!nonblockdatasock_configured)
    {
        ipc_make_noblock(datasock, &nonblockdatasock, this);
        nonblockdatasock_configured = true;
        ipc_event_loop_init(&loop);
    }
    else
    {
        ipc_event_loop_remove(&loop, &nonblockdatasock);
    }

    ipc_set_readcb_noblock(&nonblockdatasock, &nonblock_read);
    ipc_set_writecb_noblock(&nonblockdatasock, &nonblock_write);
    ipc_event_loop_add(&loop, &nonblockdatasock);
    ipc_event_loop_run(&loop);
}

void dataservice_isolation_test::nonblock_read(
    ipc_socket_context_t*, int, void* ctx)
{
    dataservice_isolation_test* that = (dataservice_isolation_test*)ctx;

    that->onRead();
}

void dataservice_isolation_test::nonblock_write(
    ipc_socket_context_t*, int, void* ctx)
{
    dataservice_isolation_test* that = (dataservice_isolation_test*)ctx;

    that->onWrite();
}

const size_t CERT_MAX_SIZE = 16384;

int dataservice_isolation_test::create_dummy_transaction(
    const uint8_t* txn_id, const uint8_t* prev_txn_id,
    const uint8_t* artifact_id, uint8_t** cert, size_t* cert_length)
{
    vccert_builder_context_t builder;
    int retval = 0;

    /* create the builder. */
    retval = vccert_builder_init(&builder_opts, &builder, CERT_MAX_SIZE);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 1;
        goto done;
    }

    /* add the certificate version. */
    retval = vccert_builder_add_short_uint32(
        &builder, VCCERT_FIELD_TYPE_CERTIFICATE_VERSION, 0x00010000);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 2;
        goto dispose_builder;
    }

    /* add the crypto suite. */
    retval = vccert_builder_add_short_uint16(
        &builder, VCCERT_FIELD_TYPE_CERTIFICATE_CRYPTO_SUITE, 0x0001);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 3;
        goto dispose_builder;
    }

    /* add the certificate type as transaction type. */
    retval = vccert_builder_add_short_UUID(
        &builder, VCCERT_FIELD_TYPE_CERTIFICATE_TYPE,
        vccert_certificate_type_uuid_txn);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 4;
        goto dispose_builder;
    }

    /* add the transaction type for this dummy transaction. */
    retval = vccert_builder_add_short_UUID(
        &builder, VCCERT_FIELD_TYPE_TRANSACTION_TYPE, dummy_transaction_type);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 5;
        goto dispose_builder;
    }

    /* add the transaction id for this dummy transaction. */
    retval = vccert_builder_add_short_UUID(
        &builder, VCCERT_FIELD_TYPE_CERTIFICATE_ID, txn_id);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 6;
        goto dispose_builder;
    }

    /* add the previous transaction id for this dummy transaction. */
    retval = vccert_builder_add_short_UUID(
        &builder, VCCERT_FIELD_TYPE_PREVIOUS_CERTIFICATE_ID, prev_txn_id);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 7;
        goto dispose_builder;
    }

    /* add the artifact id for this dummy transaction. */
    retval = vccert_builder_add_short_UUID(
        &builder, VCCERT_FIELD_TYPE_ARTIFACT_ID, artifact_id);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 8;
        goto dispose_builder;
    }

    /* emit the certificate. */
    const uint8_t* certbegin;
    size_t certsize;
    certbegin = vccert_builder_emit(&builder, &certsize);

    /* allocate memory for the built certificate. */
    *cert = (uint8_t*)malloc(certsize);
    if (NULL == *cert)
    {
        retval = 9;
        goto dispose_builder;
    }

    /* copy the certificate data into the certificate buffer. */
    memcpy(*cert, certbegin, certsize);
    *cert_length = certsize;

    /* success. */
    retval = 0;

dispose_builder:
    dispose((disposable_t*)&builder);

done:
    return retval;
}

int create_dummy_block_for_isolation(
    vccert_builder_options_t* builder_opts,
    const uint8_t* block_uuid, const uint8_t* prev_block_uuid,
    uint64_t block_height,
    uint8_t** block_cert, size_t* block_cert_length, ...)
{
    vccert_builder_context_t builder;
    int retval = 0;

    /* create the builder. */
    retval = vccert_builder_init(builder_opts, &builder, CERT_MAX_SIZE);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 1;
        goto done;
    }

    /* add the certificate version. */
    retval = vccert_builder_add_short_uint32(
        &builder, VCCERT_FIELD_TYPE_CERTIFICATE_VERSION, 0x00010000);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 2;
        goto dispose_builder;
    }

    /* add the crypto suite. */
    retval = vccert_builder_add_short_uint16(
        &builder, VCCERT_FIELD_TYPE_CERTIFICATE_CRYPTO_SUITE, 0x0001);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 3;
        goto dispose_builder;
    }

    /* add the certificate type as transaction type. */
    retval = vccert_builder_add_short_UUID(
        &builder, VCCERT_FIELD_TYPE_CERTIFICATE_TYPE,
        vccert_certificate_type_uuid_txn_block);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 4;
        goto dispose_builder;
    }

    /* add the block id for this block. */
    retval = vccert_builder_add_short_UUID(
        &builder, VCCERT_FIELD_TYPE_BLOCK_UUID, block_uuid);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 5;
        goto dispose_builder;
    }

    /* add the previous block id for this block. */
    retval = vccert_builder_add_short_UUID(
        &builder, VCCERT_FIELD_TYPE_PREVIOUS_BLOCK_UUID, prev_block_uuid);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 6;
        goto dispose_builder;
    }

    /* add the block height for this block. */
    retval = vccert_builder_add_short_uint64(
        &builder, VCCERT_FIELD_TYPE_BLOCK_HEIGHT, block_height);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 7;
        goto dispose_builder;
    }

    va_list txn_list;

    va_start(txn_list, block_cert_length);
    for (;;)
    {
        const uint8_t* txn = va_arg(txn_list, const uint8_t*);
        size_t txn_size = va_arg(txn_list, size_t);

        if (txn == NULL)
            break;

        retval = vccert_builder_add_short_buffer(
            &builder, VCCERT_FIELD_TYPE_WRAPPED_TRANSACTION_TUPLE, txn,
            txn_size);
        if (VCCERT_STATUS_SUCCESS != retval)
        {
            retval = 8;
            goto dispose_txnlist;
        }
    }

    /* emit the certificate. */
    const uint8_t* certbegin;
    size_t certsize;
    certbegin = vccert_builder_emit(&builder, &certsize);

    /* allocate memory for the built block. */
    *block_cert = (uint8_t*)malloc(certsize);
    if (NULL == *block_cert)
    {
        retval = 8;
        goto dispose_txnlist;
    }

    /* copy the certificate data into the certificate buffer. */
    memcpy(*block_cert, certbegin, certsize);
    *block_cert_length = certsize;

    /* success. */
    retval = 0;

dispose_txnlist:
    va_end(txn_list);

dispose_builder:
    dispose((disposable_t*)&builder);

done:
    return retval;
}
