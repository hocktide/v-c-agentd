/**
 * \file test_authservice_isolation.cpp
 *
 * Isolation tests for the auth service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/authservice/api.h>
#include <agentd/authservice/private/authservice.h>
#include <agentd/status_codes.h>
#include <unistd.h>
#include <vpr/disposable.h>

#include "test_auth_service_isolation.h"

using namespace std;

/**
 * Test that we can spawn the auth service.
 */
TEST_F(auth_service_isolation_test, simple_spawn)
{
    ASSERT_EQ(0, auth_service_proc_status);
}


/**
 * Test that we can initialize the auth service.
 */
TEST_F(auth_service_isolation_test, initialize)
{
    uint32_t offset = 0U;
    uint32_t status = 0U;

    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;

    /* set up the initialization data */
    uint8_t* agent_id = 0;

    /* initialize the public key */
    vccrypt_buffer_t pub_key_buffer;
    ASSERT_EQ(0, vccrypt_buffer_init(&pub_key_buffer, &alloc_opts, 32));

    /* initialize the private key */
    vccrypt_buffer_t priv_key_buffer;
    ASSERT_EQ(0, vccrypt_buffer_init(&priv_key_buffer, &alloc_opts, 32));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    auth_service_api_recvresp_initialize(
                        &nonblockauthsock, &offset, &status);
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
                sendreq_status = auth_service_api_sendreq_initialize(
                    &nonblockauthsock, agent_id, &pub_key_buffer,
                    &priv_key_buffer);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    EXPECT_EQ(0U, offset);
    EXPECT_EQ(0U, status);
}
