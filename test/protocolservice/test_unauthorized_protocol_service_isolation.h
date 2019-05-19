/**
 * \file test_unauthorized_protocol_service_isolation.h
 *
 * Private header for the unauthorized data service isolation tests.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#ifndef TEST_UNAUTHORIZED_PROTOCOL_SERVICE_ISOLATION_HEADER_GUARD
#define TEST_UNAUTHORIZED_PROTOCOL_SERVICE_ISOLATION_HEADER_GUARD

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
 * The unauthorized protocol service isolation test class deals with the
 * drudgery of communicating with the unauthorized protocol service.  It
 * provides a registration mechanism so that data can be sent to the service and
 * received from the service.
 */
class unauthorized_protocol_service_isolation_test : public ::testing::Test, public directory_test_helper {
protected:
    /* Google Test overrides. */
    void SetUp() override;
    void TearDown() override;

    bootstrap_config_t bconf;
    agent_config_t conf;
    int acceptsock;
    int datasock;
    int logsock;
    int protosock;
    pid_t protopid;
    int proto_proc_status;
    char* path;
    char wd[16384];
    const char* oldpath;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    bool suite_instance_initialized;
    bool suite_initialized;
    vccrypt_buffer_t client_private_key;
    bool client_private_key_initialized;

    static const uint8_t dir_key[32];
    static const uint8_t authorized_entity_id[16];
    static const char* authorized_entity_id_string;
    static const uint8_t authorized_entity_pubkey[32];
    static const char* authorized_entity_pubkey_string;
    static const uint8_t authorized_entity_privkey[32];
    static const uint8_t agent_id[16];
    static const char* agent_id_string;
    static const uint8_t agent_pubkey[32];
    static const char* agent_pubkey_string;
    static const uint8_t agent_privkey[32];
    static const char* agent_privkey_string;
};

#endif /*TEST_UNAUTHORIZED_PROTOCOL_SERVICE_ISOLATION_HEADER_GUARD*/
