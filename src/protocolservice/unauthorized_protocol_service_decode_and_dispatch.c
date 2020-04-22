/**
 * \file protocolservice/unauthorized_protocol_service_decode_and_dispatch.c
 *
 * \brief Decode and dispatch a command from the client.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Decode and dispatch a request from the client.
 *
 * \param conn              The connection to close.
 * \param request_id        The request ID to decode.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_decode_and_dispatch(
    unauthorized_protocol_connection_t* conn, uint32_t request_id,
    uint32_t request_offset, const uint8_t* breq, size_t size)
{
    /* save the request id. */
    conn->request_id = request_id;

    /* decode the request id */
    switch (request_id)
    {
        case UNAUTH_PROTOCOL_REQ_ID_LATEST_BLOCK_ID_GET:
            unauthorized_protocol_service_handle_request_latest_block_id_get(
                conn, request_offset, breq, size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_CLOSE:
            unauthorized_protocol_service_error_response(
                conn, request_id, AGENTD_STATUS_SUCCESS, request_offset, true);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_SUBMIT:
            unauthorized_protocol_service_handle_request_transaction_submit(
                conn, request_offset, breq, size);
            break;

        /* TODO - replace with valid error code. */
        default:
            unauthorized_protocol_service_error_response(
                conn, request_id, 8675309, request_offset, true);
    }
}
