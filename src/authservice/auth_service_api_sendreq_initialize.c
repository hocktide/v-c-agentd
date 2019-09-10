/**
 * \file authservice/auth_service_api_sendreq_initialize.c
 *
 * \brief Initialize the auth service by setting the UUID, public and private
 * keys.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/authservice/api.h>
#include <agentd/authservice/private/authservice.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Initialize the auth service.
 * 
 * Initialize by setting the UUID, public and private keys.
 *
 * \param sock          The socket on which this request is made.
 * \param agent_id      The agent's ID (UUID)
 * \param pub_key       The agent's public key
 * \param priv_key      The agent's private key
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if this write operation would block this
 *        thread.
 *      - AGENTD_ERROR_AUTHSERVICE_IPC_WRITE_DATA_FAILURE if an error occurred
 *        when writing to the socket.
 */
int auth_service_api_sendreq_initialize(
    ipc_socket_context_t* sock, const uint8_t* UNUSED(agent_id),
    const vccrypt_buffer_t* pub_key, const vccrypt_buffer_t* priv_key)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != agent_id);
    MODEL_ASSERT(NULL != pub_key);
    MODEL_ASSERT(pub_key->size > 0);
    MODEL_ASSERT(NULL != priv_key);
    MODEL_ASSERT(priv_key->size > 0);

    /* | Auth service init request packet.                              | */
    /* | --------------------------------------------- | -------------- | */
    /* | DATA                                          | SIZE           | */
    /* | --------------------------------------------- | -------------- | */
    /* | AUTHSERVICE_API_METHOD_INITIALIZE             | 4 bytes        | */
    /* | agent_id                                      | 16 bytes       | */
    /* | pub_key                                       | pub_key->size  | */
    /* | priv_key                                      | priv_key->size | */
    /* | --------------------------------------------- | -------------- | */

    /* allocate a structure large enough for writing this request. */
    size_t reqbuflen = sizeof(uint32_t) +
        16 + /* agent_id */
        pub_key->size +
        priv_key->size;

    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(AUTHSERVICE_API_METHOD_INITIALIZE);
    memcpy(reqbuf, &req, sizeof(req));

    /* TODO: write remaining data to buffer */
    memset(reqbuf + 4, 0, reqbuflen - 4);

    /* write out the request buffer */
    int retval = ipc_write_data_noblock(sock, reqbuf, reqbuflen);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK != retval && AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_AUTHSERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up memory. */
    memset(reqbuf, 0, reqbuflen);
    free(reqbuf);


    return retval;
}
