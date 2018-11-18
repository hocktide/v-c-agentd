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
};

#endif /*TEST_DATASERVICE_ISOLATION_HEADER_GUARD*/
