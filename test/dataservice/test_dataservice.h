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
#include <vccert/builder.h>
#include <vccrypt/block_cipher.h>
#include <vccrypt/suite.h>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/disposable.h>

#include "../../src/dataservice/dataservice_internal.h"
#include "../directory_test_helper.h"

class dataservice_test : public ::testing::Test, public directory_test_helper {
protected:
    void SetUp() override;
    void TearDown() override;
    int create_dummy_transaction(
        const uint8_t* txn_id, const uint8_t* prev_txn_id,
        const uint8_t* artifact_id, uint8_t** cert, size_t* cert_length);

    int suite_init_result;
    int builder_opts_init_result;
    allocator_options_t alloc_opts;
    vccert_builder_options_t builder_opts;
    vccrypt_suite_options_t crypto_suite;

    static const uint8_t dummy_artifact_type[16];
    static const uint8_t dummy_transaction_type[16];
    static const uint8_t zero_uuid[16];

private:
    static const uint8_t dir_key[32];
};

extern "C" {
int create_dummy_block(
    vccert_builder_options_t* builder_opts,
    const uint8_t* block_uuid, const uint8_t* prev_block_uuid,
    uint64_t block_height,
    uint8_t** block_cert, size_t* block_cert_length, ...);
}

#endif /*TEST_DATASERVICE_HEADER_GUARD*/
