/**
 * \file test_consensusservice_isolation.h
 *
 * Private header for the consensus service isolation tests.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#ifndef TEST_CONSENSUSSERVICE_ISOLATION_HEADER_GUARD
#define TEST_CONSENSUSSERVICE_ISOLATION_HEADER_GUARD

#include "../mocks/dataservice.h"
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
 * The consensus service isolation test class deals with the drudgery of
 * communicating with the consensus service.  It provides a registration
 * mechanism so that data can be sent to the service and received from the
 * service.
 */
class consensusservice_isolation_test : public ::testing::Test {
protected:
    /* Google Test overrides. */
    void SetUp() override;
    void TearDown() override;

    bootstrap_config_t bconf;
    agent_config_t conf;
    int datasock;
    int controlsock;
    int logsock;
    int rlogsock;
    pid_t consensuspid;
    int consensus_proc_status;
    char* path;
    char wd[16384];
    const char* oldpath;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    bool suite_instance_initialized;
    bool suite_initialized;
    std::unique_ptr<mock_dataservice::mock_dataservice> dataservice;

    static const uint8_t dir_key[32];

    static const uint32_t EXPECTED_CHILD_INDEX;

    /** \brief Helper to register dataservice boilerplate methods. */
    int dataservice_mock_register_helper();

    /** \brief Helper to verify dataservice calls on connection setup. */
    int dataservice_mock_valid_connection_setup();

    /** \brief Helper to verify dataservice calls on connection teardown. */
    int dataservice_mock_valid_connection_teardown();
};

#endif /*TEST_CONSENSUSSERVICE_ISOLATION_HEADER_GUARD*/
