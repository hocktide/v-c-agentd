/**
 * \file protocolservice/unauthorized_protocol_service_handle_request_status_get.c
 *
 * \brief Handle a status get request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Handle a status get request.
 *
 * \param conn              The connection.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_status_get(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* UNUSED(breq), size_t UNUSED(size))
{
    int retval = AGENTD_STATUS_SUCCESS;

    /* build the payload. */
    uint32_t payload[3] = {
        htonl(UNAUTH_PROTOCOL_REQ_ID_STATUS_GET), 
        htonl(retval),
        htonl(request_offset) };

    /* write the response. */
    retval =
        ipc_write_authed_data_noblock(
            &conn->ctx, conn->server_iv, payload, sizeof(payload),
            &conn->svc->suite, &conn->shared_secret);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_close_connection(conn);
        return;
    }

    /* update the server iv. */
    ++conn->server_iv;

    /* evolve connection state. */
    conn->state = APCS_WRITE_COMMAND_RESP_TO_CLIENT;

    /* set the write callback for the protocol socket. */
    ipc_set_writecb_noblock(
        &conn->ctx, &unauthorized_protocol_service_connection_write,
        &conn->svc->loop);
}
