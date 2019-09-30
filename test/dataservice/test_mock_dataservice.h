/**
 * \file test_mock_dataservice.h
 *
 * Private header for the mock dataservice unit tests.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#ifndef TEST_MOCK_DATASERVICE_HEADER_GUARD
#define TEST_MOCK_DATASERVICE_HEADER_GUARD

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
#include "../mocks/dataservice.h"

class mock_dataservice_test : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    int create_dummy_transaction(
        const uint8_t* txn_id, const uint8_t* prev_txn_id,
        const uint8_t* artifact_id, uint8_t** cert, size_t* cert_length);

    std::unique_ptr<mock_dataservice::mock_dataservice> mock;
    int datasock;
    int suite_init_result;
    int builder_opts_init_result;
    allocator_options_t alloc_opts;
    vccert_builder_options_t builder_opts;
    vccrypt_suite_options_t crypto_suite;

    void nonblockmode(
        std::function<void()> onRead, std::function<void()> onWrite);
    static void nonblock_read(ipc_socket_context_t*, int, void* ctx);
    static void nonblock_write(ipc_socket_context_t*, int, void* ctx);

    static const uint8_t dummy_artifact_type[16];
    static const uint8_t dummy_transaction_type[16];
    static const uint8_t zero_uuid[16];
    ipc_socket_context_t nonblockdatasock;
    bool nonblockdatasock_configured;
    ipc_event_loop_context_t loop;
    std::function<void()> onRead;
    std::function<void()> onWrite;
};

extern "C" {
int create_dummy_block(
    vccert_builder_options_t* builder_opts,
    const uint8_t* block_uuid, const uint8_t* prev_block_uuid,
    uint64_t block_height,
    uint8_t** block_cert, size_t* block_cert_length, ...);
}

#endif /*TEST_MOCK_DATASERVICE_HEADER_GUARD*/
