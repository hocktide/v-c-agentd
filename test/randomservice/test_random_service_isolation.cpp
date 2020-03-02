/**
 * \file test_random_service_isolation.cpp
 *
 * Isolation tests for the random service.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vpr/disposable.h>

#include "test_random_service_isolation.h"

using namespace std;

/**
 * Test that we can spawn the random service.
 */
TEST_F(random_service_isolation_test, simple_spawn)
{
    ASSERT_EQ(0, random_proc_status);
}

/**
 * Test that we can get one byte of random data from the random service.
 */
TEST_F(random_service_isolation_test, one_byte_blocking)
{
    const uint32_t EXPECTED_OFFSET = 17U;
    uint32_t offset, status;
    void* random_byte_buffer = nullptr;
    size_t random_byte_buffer_size = 0UL;

    /* send a blocking request to get random bytes. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        random_service_api_sendreq_random_bytes_get_block(
            rprotosock, EXPECTED_OFFSET, 1));

    /* receive a blocking response to get random bytes. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        random_service_api_recvresp_random_bytes_get_block(
            rprotosock, &offset, &status, &random_byte_buffer,
            &random_byte_buffer_size));

    /* verify offset, status, and size. */
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    EXPECT_EQ(1U, random_byte_buffer_size);
}

/**
 * Test that we can get many bytes of random data from the random service.
 */
TEST_F(random_service_isolation_test, many_bytes_blocking)
{
    const uint32_t EXPECTED_OFFSET = 17U;
    uint32_t offset, status;
    void* random_byte_buffer = nullptr;
    size_t random_byte_buffer_size = 0UL;

    /* send a blocking request to get random bytes. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        random_service_api_sendreq_random_bytes_get_block(
            rprotosock, EXPECTED_OFFSET, 100));

    /* receive a blocking response to get random bytes. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        random_service_api_recvresp_random_bytes_get_block(
            rprotosock, &offset, &status, &random_byte_buffer,
            &random_byte_buffer_size));

    /* verify offset, status, and size. */
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    EXPECT_EQ(100U, random_byte_buffer_size);
}

/**
 * Test that we can get one byte of random data from the random service.
 */
TEST_F(random_service_isolation_test, one_byte)
{
    const uint32_t EXPECTED_OFFSET = 17U;
    uint32_t offset, status;
    void* random_byte_buffer = nullptr;
    size_t random_byte_buffer_size = 0UL;

    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    random_service_api_recvresp_random_bytes_get(
                        &nonblockrandomsock, &offset, &status,
                        &random_byte_buffer, &random_byte_buffer_size);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    random_service_api_sendreq_random_bytes_get(
                        &nonblockrandomsock, EXPECTED_OFFSET, 1);
            }
        });

    /* verify the send request status. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);

    /* verify offset, status, and size. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(1U, random_byte_buffer_size);
}

/**
 * Test that we can get many bytes of random data from the random service.
 */
TEST_F(random_service_isolation_test, many_bytes)
{
    const uint32_t EXPECTED_OFFSET = 17U;
    uint32_t offset, status;
    void* random_byte_buffer = nullptr;
    size_t random_byte_buffer_size = 0UL;

    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    random_service_api_recvresp_random_bytes_get(
                        &nonblockrandomsock, &offset, &status,
                        &random_byte_buffer, &random_byte_buffer_size);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    random_service_api_sendreq_random_bytes_get(
                        &nonblockrandomsock, EXPECTED_OFFSET, 100);
            }
        });

    /* verify the send request status. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);

    /* verify offset, status, and size. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    EXPECT_EQ(EXPECTED_OFFSET, offset);
    EXPECT_EQ(100U, random_byte_buffer_size);
}
