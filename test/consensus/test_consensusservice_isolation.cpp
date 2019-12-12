/**
 * \file test_consensusservice_isolation.cpp
 *
 * Isolation tests for the consensus service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <agentd/consensusservice/api.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vpr/disposable.h>

#include "test_consensusservice_isolation.h"

/**
 * Test that we can spawn the consensus service.
 */
TEST_F(consensusservice_isolation_test, simple_spawn)
{
    ASSERT_EQ(0, consensus_proc_status);
}

/**
 * Test that calling start before calling configure results in an error.
 */
TEST_F(consensusservice_isolation_test, start_before_configure_fail)
{
    uint32_t offset = 0, status = AGENTD_STATUS_SUCCESS;

    /* we should be able to successfully call start. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        consensus_api_sendreq_start(controlsock));

    /* we should be able to receive a response from the start call. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        consensus_api_recvresp_start(
            controlsock, &offset, &status));

    /* the status should NOT be success. */
    ASSERT_EQ(
        AGENTD_ERROR_CONSENSUSSERVICE_START_BEFORE_CONFIGURE, (int)status);
}

/**
 * Test that we can configure the consensus service.
 */
TEST_F(consensusservice_isolation_test, configure)
{
    agent_config_t conf;
    uint32_t offset = 999, status = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;

    /* set config values for consensus service. */
    conf.block_max_seconds_set = true;
    conf.block_max_seconds = 2;
    conf.block_max_transactions_set = true;
    conf.block_max_transactions = 1000;

    /* we should be able to successfully call config. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        consensus_api_sendreq_configure(controlsock, &conf));

    /* we should be able to receive a response from config. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        consensus_api_recvresp_configure(controlsock, &offset, &status));

    /* the status should be success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    EXPECT_EQ(0U, offset);
}

/**
 * Test that we can start the consensus service after configuring it.
 */
TEST_F(consensusservice_isolation_test, start)
{
    agent_config_t conf;
    uint32_t offset = 999, status = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;

    /* set config values for consensus service. */
    conf.block_max_seconds_set = true;
    conf.block_max_seconds = 2;
    conf.block_max_transactions_set = true;
    conf.block_max_transactions = 1000;

    /* we should be able to successfully call config. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        consensus_api_sendreq_configure(controlsock, &conf));

    /* we should be able to receive a response from config. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        consensus_api_recvresp_configure(controlsock, &offset, &status));

    /* the status should be success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    EXPECT_EQ(0U, offset);

    /* we should be able to successfully call start. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        consensus_api_sendreq_start(controlsock));

    /* we should be able to receive a response from the start call. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        consensus_api_recvresp_start(
            controlsock, &offset, &status));

    /* the status should be success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    EXPECT_EQ(0U, offset);
}
