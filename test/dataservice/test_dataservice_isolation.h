/**
 * \file test_dataservice_isolation.h
 *
 * Private header for the dataservice isolation tests.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#ifndef TEST_DATASERVICE_ISOLATION_HEADER_GUARD
#define TEST_DATASERVICE_ISOLATION_HEADER_GUARD

#include "../directory_test_helper.h"
#include <agentd/config.h>
#include <agentd/inet.h>
#include <agentd/ipc.h>
#include <agentd/string.h>
#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <vccert/builder.h>
#include <vccrypt/block_cipher.h>
#include <vccrypt/suite.h>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/disposable.h>

extern "C" {
#include <config/agentd.tab.h>
#include <config/agentd.yy.h>
}

/* this header will only work for C++. */
#if !defined(__cplusplus)
#error This is a C++ header file.
#endif /*! defined(__cplusplus)*/

/**
 * \brief Simple user context structure for testing.
 */
struct test_context
{
    disposable_t hdr;
    std::vector<std::string> errors;
    agent_config_t* config;
};

/**
 * The dataservice isolation test class deals with the drudgery of communicating
 * with the data service.  It provides a registration mechanism so that
 * data can be sent to the data service and received from the data service.
 */
class dataservice_isolation_test : public ::testing::Test, public directory_test_helper {
protected:
    /* Google Test overrides. */
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

    void nonblockmode(
        std::function<void()> onRead, std::function<void()> onWrite);
    static void nonblock_read(ipc_socket_context_t*, int, void* ctx);
    static void nonblock_write(ipc_socket_context_t*, int, void* ctx);

    static void test_context_dispose(void* disp);
    static void test_context_init(test_context* ctx);
    static void set_error(config_context_t* context, const char* msg);
    static void config_callback(
        config_context_t* context, agent_config_t* config);

    bootstrap_config_t bconf;
    int datasock;
    int logsock;
    pid_t datapid;
    int dataservice_proc_status;
    char* path;
    char wd[16384];
    const char* oldpath;
    ipc_socket_context_t nonblockdatasock;
    bool nonblockdatasock_configured;
    ipc_event_loop_context_t loop;
    std::function<void()> onRead;
    std::function<void()> onWrite;

    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    static const uint8_t dir_key[32];
    static const uint8_t dummy_artifact_type[16];
    static const uint8_t dummy_transaction_type[16];
    static const uint8_t zero_uuid[16];
};

extern "C" {
int create_dummy_block_for_isolation(
    vccert_builder_options_t* builder_opts,
    const uint8_t* block_uuid, const uint8_t* prev_block_uuid,
    uint64_t block_height,
    uint8_t** block_cert, size_t* block_cert_length, ...);
}

#endif /*TEST_DATASERVICE_ISOLATION_HEADER_GUARD*/
