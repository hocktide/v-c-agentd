/**
 * \file test_dataservice_helpers.cpp
 *
 * Helpers for the dataservice unit test.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include "test_dataservice.h"

using namespace std;

const uint8_t dataservice_test::dir_key[32] = {
    0xe6, 0x17, 0xb1, 0x70, 0xa9, 0xfa, 0x40, 0x72,
    0xa9, 0x0a, 0x25, 0x3b, 0x23, 0x75, 0x34, 0x23,
    0xc0, 0x5d, 0x71, 0x59, 0x74, 0x7d, 0x40, 0x8e,
    0xb9, 0x01, 0x8e, 0x2c, 0xdd, 0x96, 0x38, 0x55
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
}

void dataservice_test::TearDown()
{
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

    /* zeroes used for IV to force ECB mode. */
    uint8_t zeroes[16] = {
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
    };

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
        vccrypt_block_encrypt(&ctx, zeroes, inBuf, outBuf))
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
        vccrypt_block_encrypt(&ctx, zeroes, inBuf, outBuf + 16))
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
