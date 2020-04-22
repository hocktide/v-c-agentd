/**
 * \file protocolservice/unauthorized_protocol_service_handle_request_latest_block_id_get.c
 *
 * \brief Handle the latest block id get request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Handle a latest_block_id_get request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_latest_block_id_get(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* UNUSED(breq), size_t UNUSED(size))
{
    int retval;

    /* save the request offset. */
    conn->current_request_offset = request_offset;

    /* wait on the response from the "app" (dataservice) */
    conn->state = APCS_READ_COMMAND_RESP_FROM_APP;

    /* write the request to the dataservice using our child context. */
    retval =
        dataservice_api_sendreq_latest_block_id_get(
            &conn->svc->data, conn->dataservice_child_context);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_error_response(
            conn, UNAUTH_PROTOCOL_REQ_ID_LATEST_BLOCK_ID_GET,
            retval,
            request_offset, true);
        return;
    }

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        &conn->svc->data, &unauthorized_protocol_service_dataservice_write,
        &conn->svc->loop);
}
