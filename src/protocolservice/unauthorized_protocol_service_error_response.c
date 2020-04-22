/**
 * \file protocolservice/unauthorized_protocol_service_error_response.c
 *
 * \brief Send an error response to the client and start the disconnection
 * process.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Write an error response to the socket and set the connection state to
 * unauthorized.
 *
 * This method writes an error response to the socket and sets up the state
 * machine to close the connection after the error response is written.
 *
 * \param conn          The connection to which the error response is written.
 * \param request_id    The id of the request that caused the error.
 * \param status        The status code of the error.
 * \param offset        The request offset that caused the error.
 * \param encrypted     Set to true if this packet should be encrypted.
 */
void unauthorized_protocol_service_error_response(
    unauthorized_protocol_connection_t* conn, int request_id, int status,
    uint32_t offset, bool encrypted)
{
    int retval = AGENTD_STATUS_SUCCESS;

    uint32_t payload[3] = { htonl(request_id), htonl(status), htonl(offset) };

    /* attempt to write the response payload to the socket. */
    if (encrypted)
    {
        /* encrypted write. */
        retval =
            ipc_write_authed_data_noblock(
                &conn->ctx, conn->server_iv, payload, sizeof(payload),
                &conn->svc->suite, &conn->shared_secret);

        /* Update the server iv. */
        ++conn->server_iv;
    }
    else
    {
        /* unencrypted write. */
        retval = ipc_write_data_noblock(&conn->ctx, payload, sizeof(payload));
    }

    /* verify the status of writing this error response. */
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_close_connection(conn);
        return;
    }

    /* set state to unauthorized so we disconnect after writing the error. */
    conn->state = UPCS_UNAUTHORIZED;

    /* set the write callback for the protocol socket. */
    ipc_set_writecb_noblock(
        &conn->ctx, &unauthorized_protocol_service_connection_write,
        &conn->svc->loop);
}
