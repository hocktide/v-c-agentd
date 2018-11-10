/**
 * \file test_dataservice.h
 *
 * Private header for the dataservice unit tests.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#ifndef TEST_DATASERVICE_HEADER_GUARD
#define TEST_DATASERVICE_HEADER_GUARD

/* this header will only work for C++. */
#if !defined(__cplusplus)
#error This is a C++ header file.
#endif /*! defined(__cplusplus)*/

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <gtest/gtest.h>
#include <lmdb.h>
#include <string>
#include <vccrypt/block_cipher.h>
#include <vccrypt/suite.h>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/disposable.h>

#include "../../src/dataservice/dataservice_internal.h"

class dataservice_test : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    int setDirectoryName(uint64_t offset, std::string& dname);
    int createDirectoryName(uint64_t arg, std::string& dname);

    const char* dbDirPath;
    int suite_init_result;
    int dircrypt_options_init_result;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t crypto_suite;
    vccrypt_block_options_t dircrypt_options;

    static const uint8_t dir_key[32];
};

#endif /*TEST_DATASERVICE_HEADER_GUARD*/
