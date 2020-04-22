/**
 * \file
 * protocolservice/unauthorized_protocol_service_connection_handshake_ack_read.c
 *
 * \brief Read a handshake acknowledge packet from the client.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Attempt to the client challenge response acknowledgement.
 *
 * \param conn      The connection from which this ack should be read.
 */
void unauthorized_protocol_service_connection_handshake_ack_read(
    unauthorized_protocol_connection_t* conn)
{
    void* req = NULL;
    uint32_t size = 0U;

    /* attempt to read the ack packet. */
    int retval =
        ipc_read_authed_data_noblock(
            &conn->ctx, conn->client_iv, &req, &size, &conn->svc->suite,
            &conn->shared_secret);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK == retval)
    {
        ipc_set_readcb_noblock(
            &conn->ctx, &unauthorized_protocol_service_connection_read,
            &conn->svc->loop);
        return;
    }
    if (AGENTD_ERROR_IPC_EVBUFFER_READ_FAILURE == retval || AGENTD_ERROR_IPC_EVBUFFER_EOF == retval)
    {
        unauthorized_protocol_service_close_connection(conn);
        return;
    }
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_error_response(
            conn, 0, AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0, true);
        return;
    }

    /* from here on, we are committed.  Don't call this callback again. */
    ++conn->client_iv;
    ipc_set_readcb_noblock(&conn->ctx, NULL, &conn->svc->loop);

    /* Build the ack payload. */
    uint32_t payload[3] = {
        htonl(UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_ACKNOWLEDGE),
        htonl(AGENTD_STATUS_SUCCESS),
        htonl(0)
    };

    /* attempt to write this payload to the socket. */
    retval =
        ipc_write_authed_data_noblock(
            &conn->ctx, conn->server_iv, payload, sizeof(payload),
            &conn->svc->suite, &conn->shared_secret);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_close_connection(conn);
        goto cleanup_data;
    }

    /* Update the server iv on success. */
    ++conn->server_iv;

    /* set the updated connection state. */
    conn->state = UPCS_WRITE_HANDSHAKE_ACK_TO_CLIENT;

    /* set the write callback. */
    ipc_set_writecb_noblock(
        &conn->ctx, &unauthorized_protocol_service_connection_write,
        &conn->svc->loop);

    /* success. */

cleanup_data:
    memset(req, 0, size);
    free(req);
}
