/**
 * \file test_random_service_isolation.h
 *
 * Private header for the random service isolation tests.
 *
 * \copyright 2020 Velo-Payments, Inc.  All rights reserved.
 */

#ifndef TEST_RANDOM_SERVICE_ISOLATION_HEADER_GUARD
#define TEST_RANDOM_SERVICE_ISOLATION_HEADER_GUARD

#include <agentd/inet.h>
#include <agentd/ipc.h>
#include <agentd/string.h>
#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <vpr/disposable.h>

/* this header will only work for C++. */
#if !defined(__cplusplus)
#error This is a C++ header file.
#endif /*! defined(__cplusplus)*/

/**
 * The random service isolation test class deals with the drudgery of
 * communicating with the random service.  It provides a registration mechanism
 * so that data can be sent to the service and received from the service.
 */
class random_service_isolation_test : public ::testing::Test {
protected:
    /* Google Test overrides. */
    void SetUp() override;
    void TearDown() override;

    bootstrap_config_t bconf;
    agent_config_t conf;
    char* path;
    char wd[16384];
    const char* oldpath;
    int rlogsock;
    int rprotosock;
    pid_t randompid;
    int random_proc_status;
    ipc_socket_context_t nonblockrandomsock;
    bool nonblockrandomsock_configured;
    ipc_event_loop_context_t loop;
    std::function<void()> onRead;
    std::function<void()> onWrite;

    void nonblockmode(
        std::function<void()> onRead, std::function<void()> onWrite);
    static void nonblock_read(ipc_socket_context_t*, int, void* ctx);
    static void nonblock_write(ipc_socket_context_t*, int, void* ctx);
};

#endif /*TEST_RANDOM_SERVICE_ISOLATION_HEADER_GUARD*/
