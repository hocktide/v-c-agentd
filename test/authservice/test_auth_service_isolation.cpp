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
 * Test that we can initialize the auth service using BLOCKING calls.
 */
TEST_F(auth_service_isolation_test, initialize_blocking)
{
    ASSERT_EQ(0, auth_service_proc_status);

    uint32_t offset = 0U;
    uint32_t status = 0U;

    /* initialize the agent ID buffer  */
    vccrypt_buffer_t agent_id_buffer;
    size_t sz_agent_id = sizeof(agent_id);
    ASSERT_EQ(0, vccrypt_buffer_init(&agent_id_buffer, &alloc_opts, sz_agent_id));
    memcpy(agent_id_buffer.data, agent_id, sz_agent_id);

    /* initialize the public key buffer */
    vccrypt_buffer_t pubkey_buffer;
    size_t sz_pubkey = sizeof(agent_pubkey);
    ASSERT_EQ(0, vccrypt_buffer_init(&pubkey_buffer, &alloc_opts, sz_pubkey));
    memcpy(pubkey_buffer.data, agent_pubkey, sz_pubkey);

    /* initialize the private key buffer */
    vccrypt_buffer_t privkey_buffer;
    size_t sz_privkey = sizeof(agent_privkey);
    ASSERT_EQ(0, vccrypt_buffer_init(&privkey_buffer, &alloc_opts, sz_privkey));
    memcpy(privkey_buffer.data, agent_privkey, sz_privkey);

    ASSERT_EQ(0, auth_service_api_sendreq_initialize_block(authsock, &agent_id_buffer, &pubkey_buffer, &privkey_buffer));

    ASSERT_EQ(0, auth_service_api_recvresp_initialize_block(authsock, &offset, &status));

    EXPECT_EQ(0U, offset);
    EXPECT_EQ(0U, status);

    /* clean up */
    dispose((disposable_t*)&privkey_buffer);
    dispose((disposable_t*)&pubkey_buffer);
    dispose((disposable_t*)&agent_id_buffer);
}


/**
 * Test that we can initialize the auth service.
 */
TEST_F(auth_service_isolation_test, initialize)
{
    ASSERT_EQ(0, auth_service_proc_status);

    uint32_t offset = 0U;
    uint32_t status = 0U;

    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;

    /* initialize the agent ID buffer  */
    vccrypt_buffer_t agent_id_buffer;
    size_t sz_agent_id = sizeof(agent_id);
    ASSERT_EQ(0, vccrypt_buffer_init(&agent_id_buffer, &alloc_opts, sz_agent_id));
    memcpy(agent_id_buffer.data, agent_id, sz_agent_id);

    /* initialize the public key */
    vccrypt_buffer_t pubkey_buffer;
    size_t sz_pubkey = sizeof(agent_pubkey);
    ASSERT_EQ(0, vccrypt_buffer_init(&pubkey_buffer, &alloc_opts, sz_pubkey));
    memcpy(pubkey_buffer.data, agent_pubkey, sz_pubkey);

    /* initialize the private key */
    vccrypt_buffer_t privkey_buffer;
    size_t sz_privkey = sizeof(agent_privkey);
    ASSERT_EQ(0, vccrypt_buffer_init(&privkey_buffer, &alloc_opts, sz_privkey));
    memcpy(privkey_buffer.data, agent_privkey, sz_privkey);

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
                    &nonblockauthsock, &agent_id_buffer, &pubkey_buffer,
                    &privkey_buffer);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    EXPECT_EQ(0U, offset);
    EXPECT_EQ(0U, status);

    /* clean up */
    dispose((disposable_t*)&privkey_buffer);
    dispose((disposable_t*)&pubkey_buffer);
    dispose((disposable_t*)&agent_id_buffer);
}
