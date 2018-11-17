/**
 * \file test_dataservice_helpers.cpp
 *
 * Helpers for the dataservice unit test.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <vccert/fields.h>
#include <vccert/certificate_types.h>

#include "test_dataservice.h"

using namespace std;

const uint8_t dataservice_test::dir_key[32] = {
    0xe6, 0x17, 0xb1, 0x70, 0xa9, 0xfa, 0x40, 0x72,
    0xa9, 0x0a, 0x25, 0x3b, 0x23, 0x75, 0x34, 0x23,
    0xc0, 0x5d, 0x71, 0x59, 0x74, 0x7d, 0x40, 0x8e,
    0xb9, 0x01, 0x8e, 0x2c, 0xdd, 0x96, 0x38, 0x55
};

const uint8_t dataservice_test::dummy_artifact_type[16] = {
    0xf4, 0x1d, 0x06, 0x9c, 0xc0, 0x2d, 0x4b, 0xea,
    0xb6, 0x5c, 0x01, 0xe9, 0x48, 0xc3, 0xca, 0x11
};

const uint8_t dataservice_test::dummy_transaction_type[16] = {
    0x35, 0x3a, 0x21, 0xad, 0xc3, 0xd7, 0x4e, 0x01,
    0xaf, 0x4c, 0x90, 0x58, 0x7c, 0x68, 0xe6, 0xcf
};

const uint8_t dataservice_test::zero_uuid[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void dataservice_test::SetUp()
{
    dbDirPath = "build/host/checked/databases/";
    vccrypt_suite_register_velo_v1();
    vccrypt_block_register_AES_256_2X_CBC();

    malloc_allocator_options_init(&alloc_opts);

    suite_init_result =
        vccrypt_suite_options_init(
            &crypto_suite, &alloc_opts,
            VCCRYPT_SUITE_VELO_V1);

    dircrypt_options_init_result =
        vccrypt_block_options_init(
            &dircrypt_options, &alloc_opts,
            VCCRYPT_BLOCK_ALGORITHM_AES_256_2X_CBC);

    builder_opts_init_result =
        vccert_builder_options_init(
            &builder_opts, &alloc_opts, &crypto_suite);
}

void dataservice_test::TearDown()
{
    if (builder_opts_init_result == 0)
    {
        dispose((disposable_t*)&builder_opts);
    }

    if (suite_init_result == 0)
    {
        dispose((disposable_t*)&crypto_suite);
    }

    if (dircrypt_options_init_result == 0)
    {
        dispose((disposable_t*)&dircrypt_options);
    }

    dispose((disposable_t*)&alloc_opts);
}

int dataservice_test::createDirectoryName(uint64_t arg, string& dname)
{
    int retval = 0;

    /* set the directory path based on the arg value. */
    retval = setDirectoryName(arg, dname);
    if (retval != 0)
    {
        return retval;
    }

    /* create the command to set the directory path. */
    string cmd("mkdir -p ");
    cmd += dname;

    retval = system(cmd.c_str());
    if (retval != 0)
    {
        return retval;
    }

    return 0;
}

int dataservice_test::setDirectoryName(uint64_t offset, string& dname)
{
    int retval = 0;
    uint8_t inBuf[16];
    uint8_t outBuf[32];

    /* seed the first part of the directory name. */
    uint64_t seed = 0;
    memcpy(inBuf, &seed, sizeof(seed));
    memcpy(inBuf + sizeof(seed), &offset, sizeof(offset));

    /* create key buffer. */
    vccrypt_buffer_t keyBuf;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(&keyBuf, &alloc_opts, 32))
    {
        retval = 1;
        goto done;
    }

    /* read directory key. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_read_data(&keyBuf, dir_key, 32))
    {
        retval = 2;
        goto dispose_keyBuf;
    }

    /* create block cipher context. */
    vccrypt_block_context_t ctx;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_block_init(&dircrypt_options, &ctx, &keyBuf, true))
    {
        retval = 3;
        goto dispose_keyBuf;
    }

    /* encrypt the first block of the directory name. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_block_encrypt(&ctx, zero_uuid, inBuf, outBuf))
    {
        retval = 4;
        goto dispose_block_cipher;
    }

    /* seed the second part of the directory name. */
    ++seed;
    memcpy(inBuf, &seed, sizeof(seed));
    memcpy(inBuf + sizeof(seed), &offset, sizeof(offset));

    /* encrypt the second block of the directory name. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_block_encrypt(&ctx, zero_uuid, inBuf, outBuf + 16))
    {
        retval = 5;
        goto dispose_block_cipher;
    }

    /* create output buffer. */
    vccrypt_buffer_t out;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(&out, &alloc_opts, 32))
    {
        retval = 6;
        goto dispose_out;
    }

    /* read the directory name into this output buffer. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_read_data(&out, outBuf, 32))
    {
        retval = 7;
        goto dispose_out;
    }

    /* create the hex encode buffer. */
    vccrypt_buffer_t hexBuf;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init_for_hex_serialization(
            &hexBuf, &alloc_opts, 32))
    {
        retval = 8;
        goto dispose_out;
    }

    /* write the hex value for the directory name to the hex buffer. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_write_hex(&hexBuf, &out))
    {
        retval = 9;
        goto dispose_hex_buf;
    }

    /* create the directory name as a string. */
    char dirName[65];
    memset(dirName, 0, sizeof(dirName));
    memcpy(dirName, hexBuf.data, 64);

    /* create the complete directory path. */
    dname = dbDirPath;
    dname += dirName;

    /* success. */
    retval = 0;

dispose_hex_buf:
    dispose((disposable_t*)&hexBuf);

dispose_out:
    dispose((disposable_t*)&out);

dispose_block_cipher:
    dispose((disposable_t*)&ctx);

dispose_keyBuf:
    dispose((disposable_t*)&keyBuf);

done:
    return retval;
}

const size_t CERT_MAX_SIZE = 16384;

int dataservice_test::create_dummy_transaction(
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

int create_dummy_block(
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
