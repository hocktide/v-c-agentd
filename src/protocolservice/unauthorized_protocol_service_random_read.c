/**
 * \file protocolservice/unauthorized_protocol_service_random_read.c
 *
 * \brief Read from the random service socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/api.h>
#include <agentd/status_codes.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/* forward decls. */
static int unauthorized_protocol_service_handle_random_response(
    unauthorized_protocol_service_instance_t* svc);

/**
 * \brief Read data from the random service socket.
 *
 * \param ctx           The socket context triggering this event.
 * \param event_flags   The flags for this event.
 * \param user_context  The user context for this event.
 */
void unauthorized_protocol_service_random_read(
    ipc_socket_context_t* ctx, int UNUSED(event_flags),
    void* user_context)
{
    int retval;

    /* get the instance from the user context. */
    unauthorized_protocol_service_instance_t* svc =
        (unauthorized_protocol_service_instance_t*)user_context;

    /* no need to continue if we're shutting down. */
    if (svc->force_exit)
        return;

    /* loop over the input buffer. */
    do
    {
        retval = unauthorized_protocol_service_handle_random_response(svc);
    } while (AGENTD_STATUS_SUCCESS == retval
             && ipc_socket_readbuffer_size(ctx) > 0);
}

/**
 * \brief Handle a single response from the random service.
 *
 * \param svc               The protocol service instance.
 *
 * \returns a status code indicating success or failure.
 */
static int unauthorized_protocol_service_handle_random_response(
    unauthorized_protocol_service_instance_t* svc)
{
    void* resp = NULL;
    uint8_t* bresp = NULL;
    uint32_t resp_size = 0U;
    uint32_t size = 0U;

    uint32_t request_id;
    uint32_t request_offset;
    uint32_t status;
    int retval;

    /* attempt to read a response packet. */
    retval = ipc_read_data_noblock(&svc->random, &resp, &resp_size);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK == retval)
    {
        goto done;
    }
    /* handle general failures from the random service socket read. */
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_exit_event_loop(svc);
        goto done;
    }

    /* set up buffer pointer. */
    bresp = resp;
    size = resp_size;

    /* verify size */
    if (size < 3 * sizeof(uint32_t))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_RESPONSE;
        unauthorized_protocol_service_exit_event_loop(svc);
        goto cleanup_resp;
    }

    /* decode the method and verify. */
    memcpy(&request_id, bresp, sizeof(request_id));
    bresp += sizeof(request_id);
    size -= sizeof(request_id);
    request_id = ntohl(request_id);
    if (RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES != request_id)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_RESPONSE;
        unauthorized_protocol_service_exit_event_loop(svc);
        goto cleanup_resp;
    }

    /* decode the request offset and verify. */
    memcpy(&request_offset, bresp, sizeof(request_offset));
    bresp += sizeof(request_offset);
    size -= sizeof(request_offset);
    request_offset = ntohl(request_offset);
    if (request_offset >= svc->num_connections)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_RESPONSE;
        unauthorized_protocol_service_exit_event_loop(svc);
        goto cleanup_resp;
    }

    /* get the connection and verify that it is in the right state. */
    unauthorized_protocol_connection_t* conn =
        svc->connections + request_offset;
    if (UPCS_HANDSHAKE_GATHER_ENTROPY != conn->state)
    {
        /* has the connection already been closed? */
        if (UPCS_HANDSHAKE_GATHER_ENTROPY_CLOSED == conn->state)
        {
            retval = AGENTD_STATUS_SUCCESS;
            goto cleanup_resp;
        }

        /* force the connection closed. */
        retval = AGENTD_STATUS_SUCCESS;
        unauthorized_protocol_service_close_connection(conn);
        goto cleanup_resp;
    }

    /* get the status and verify that this request was successful. */
    memcpy(&status, bresp, sizeof(status));
    bresp += sizeof(status);
    size -= sizeof(status);
    status = ntohl(status);
    if (AGENTD_STATUS_SUCCESS != status)
    {
        /* log the error to the socket. */
        retval = AGENTD_STATUS_SUCCESS;
        unauthorized_protocol_service_error_response(
            conn, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE, status, 0, false);
        goto cleanup_resp;
    }

    /* copy payload data. */
    memcpy(conn->server_key_nonce.data, bresp, conn->server_key_nonce.size);
    bresp += conn->server_key_nonce.size;
    size -= conn->server_key_nonce.size;
    memcpy(
        conn->server_challenge_nonce.data, bresp,
        conn->server_challenge_nonce.size);

    /* write the handshake request response and walk state / callbacks. */
    if (AGENTD_STATUS_SUCCESS !=
        unauthorized_protocol_service_write_handshake_request_response(
            conn))
    {
        retval = AGENTD_STATUS_SUCCESS;
        unauthorized_protocol_service_close_connection(conn);
        goto cleanup_resp;
    }

    /* success. */

cleanup_resp:
    memset(resp, 0, resp_size);
    free(resp);

done:
    return retval;
}
