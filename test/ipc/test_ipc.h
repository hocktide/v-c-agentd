/**
 * \file test_ipc.h
 *
 * Private header for the ipc unit tests.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#ifndef TEST_IPC_HEADER_GUARD
#define IPC_HEADER_GUARD

#include <agentd/ipc.h>
#include <gtest/gtest.h>

/* this header will only work for C++. */
#if !defined(__cplusplus)
#error This is a C++ header file.
#endif /*! defined(__cplusplus)*/

#include <functional>

/**
 * The dataservice isolation test class deals with the drudgery of communicating
 * with the data service.  It provides a registration mechanism so that
 * data can be sent to the data service and received from the data service.
 */
class ipc_test : public ::testing::Test {
protected:
    /* Google Test overrides. */
    void SetUp() override;
    void TearDown() override;

    void nonblockmode(
        int datasock,
        std::function<void()> onRead, std::function<void()> onWrite);
    static void nonblock_read(ipc_socket_context_t*, int, void* ctx);
    static void nonblock_write(ipc_socket_context_t*, int, void* ctx);

    ipc_socket_context_t nonblockdatasock;
    bool nonblockdatasock_configured;
    ipc_event_loop_context_t loop;
    std::function<void()> onRead;
    std::function<void()> onWrite;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    bool suite_configured;
};

#endif /*TEST_IPC_HEADER_GUARD*/
