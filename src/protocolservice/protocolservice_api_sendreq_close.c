/**
 * \file protocolservice/protocolservice_api_sendreq_close.c
 *
 * \brief Send the close connection request.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/ipc.h>
#include <agentd/protocolservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vccrypt/compare.h>
#include <vpr/parameters.h>

/**
 * \brief Send an explicit close connection request to the protocol socket.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 *
 * This function sends a close socket request to the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_close(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret)
{
    int retval;

    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != suite);
    MODEL_ASSERT(NULL != client_iv);
    MODEL_ASSERT(NULL != shared_secret);

    /* create a buffer for holding the request. */
    size_t req_size = 2 * sizeof(uint32_t);
    vccrypt_buffer_t req;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(
            &req, suite->alloc_opts, req_size))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* populate the request. */
    uint8_t* breq = (uint8_t*)req.data;
    uint32_t net_method_id = htonl(UNAUTH_PROTOCOL_REQ_ID_CLOSE);
    uint32_t net_request_id = htonl(0UL);
    memcpy(breq, &net_method_id, sizeof(net_method_id));
    memcpy(breq + sizeof(uint32_t), &net_request_id, sizeof(net_request_id));

    /* write IPC authed request packet to the server. */
    /* TODO - shared secret parameter in ipc should be const. */
    retval =
        ipc_write_authed_data_block(
            sock, *client_iv, req.data, req.size, suite,
            (vccrypt_buffer_t*)shared_secret);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_req;
    }

    /* increment client IV. */
    *client_iv += 1;

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_req;

cleanup_req:
    dispose((disposable_t*)&req);

done:
    return retval;
}
