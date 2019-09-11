/**
 * \file test_auth_service_isolation.h
 *
 * Private header for the auth service isolation tests.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#ifndef TEST_AUTH_SERVICE_ISOLATION_HEADER_GUARD
#define TEST_AUTH_SERVICE_ISOLATION_HEADER_GUARD

#include <agentd/config.h>
#include <agentd/ipc.h>
#include <agentd/string.h>
#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/disposable.h>

/* this header will only work for C++. */
#if !defined(__cplusplus)
#error This is a C++ header file.
#endif /*! defined(__cplusplus)*/


class auth_service_isolation_test : public ::testing::Test {
protected:
    /* Google Test overrides. */
    void SetUp() override;
    void TearDown() override;

    static const uint8_t root_entity_id[16];
    static const uint8_t agent_pubkey[32];
    static const uint8_t agent_privkey[32];

    int suite_init_result;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t crypto_suite;

    void nonblockmode(
        std::function<void()> onRead, std::function<void()> onWrite);
    static void nonblock_read(ipc_socket_context_t*, int, void* ctx);
    static void nonblock_write(ipc_socket_context_t*, int, void* ctx);

    bootstrap_config_t bconf;
    agent_config_t conf;
    int authsock;
    int logsock;
    pid_t auth_pid;
    int auth_service_proc_status;
    char* path;
    char wd[16384];
    const char* oldpath;
    ipc_socket_context_t nonblockauthsock;
    bool nonblockauthsock_configured;
    ipc_event_loop_context_t loop;
    std::function<void()> onRead;
    std::function<void()> onWrite;
};

#endif /*TEST_AUTH_SERVICE_ISOLATION_HEADER_GUARD*/
