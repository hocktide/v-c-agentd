/**
 * \file test_mock_dataservice_helpers.cpp
 *
 * Helpers for the dataservice unit test.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#include <vccert/fields.h>
#include <vccert/certificate_types.h>

#include "test_mock_dataservice.h"

using namespace std;

const uint8_t mock_dataservice_test::dummy_artifact_type[16] = {
    0xf4, 0x1d, 0x06, 0x9c, 0xc0, 0x2d, 0x4b, 0xea,
    0xb6, 0x5c, 0x01, 0xe9, 0x48, 0xc3, 0xca, 0x11
};

const uint8_t mock_dataservice_test::dummy_transaction_type[16] = {
    0x35, 0x3a, 0x21, 0xad, 0xc3, 0xd7, 0x4e, 0x01,
    0xaf, 0x4c, 0x90, 0x58, 0x7c, 0x68, 0xe6, 0xcf
};

const uint8_t mock_dataservice_test::zero_uuid[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void mock_dataservice_test::SetUp()
{
    vccrypt_suite_register_velo_v1();
    vccrypt_block_register_AES_256_2X_CBC();

    malloc_allocator_options_init(&alloc_opts);

    suite_init_result =
        vccrypt_suite_options_init(
            &crypto_suite, &alloc_opts,
            VCCRYPT_SUITE_VELO_V1);

    builder_opts_init_result =
        vccert_builder_options_init(
            &builder_opts, &alloc_opts, &crypto_suite);

    /* create a socketpair for the mock and data sockets. */
    int mocksock;
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &mocksock, &datasock);

    /* create the mock dataservice. */
    mock = make_unique<mock_dataservice::mock_dataservice>(mocksock);

    /* by default, we run in blocking mode. */
    nonblockdatasock_configured = false;
}

void mock_dataservice_test::TearDown()
{
    /* destroy the mock dataservice. */
    mock.reset(nullptr);

    if (builder_opts_init_result == 0)
    {
        dispose((disposable_t*)&builder_opts);
    }

    if (suite_init_result == 0)
    {
        dispose((disposable_t*)&crypto_suite);
    }

    dispose((disposable_t*)&alloc_opts);
}

const size_t CERT_MAX_SIZE = 16384;

int mock_dataservice_test::create_dummy_transaction(
    const uint8_t* txn_id, const uint8_t* prev_txn_id,
    const uint8_t* artifact_id, uint8_t** cert, size_t* cert_length)
{
    vccert_builder_context_t builder;
    uint32_t prev_state = 0xFFFFFFFF;
    uint32_t new_state = 0x00000000;
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

    /* add the previous state. */
    retval = vccert_builder_add_short_uint32(
        &builder, VCCERT_FIELD_TYPE_PREVIOUS_ARTIFACT_STATE, prev_state);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 8;
        goto dispose_builder;
    }

    /* add the new state. */
    retval = vccert_builder_add_short_uint32(
        &builder, VCCERT_FIELD_TYPE_NEW_ARTIFACT_STATE, new_state);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 9;
        goto dispose_builder;
    }

    /* add the artifact id for this dummy transaction. */
    retval = vccert_builder_add_short_UUID(
        &builder, VCCERT_FIELD_TYPE_ARTIFACT_ID, artifact_id);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        retval = 10;
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
        retval = 11;
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

void mock_dataservice_test::nonblockmode(
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

void mock_dataservice_test::nonblock_read(
    ipc_socket_context_t*, int, void* ctx)
{
    mock_dataservice_test* that = (mock_dataservice_test*)ctx;

    that->onRead();
}

void mock_dataservice_test::nonblock_write(
    ipc_socket_context_t*, int, void* ctx)
{
    mock_dataservice_test* that = (mock_dataservice_test*)ctx;

    that->onWrite();
}
