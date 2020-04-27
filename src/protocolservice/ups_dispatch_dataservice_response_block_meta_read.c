/**
 * \file protocolservice/ups_dispatch_dataservice_response_block_meta_read.c
 *
 * \brief Handle the response from the dataservice block read request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * Handle a meta block read response.
 *
 * \param svc               The protocol service instance.
 * \param resp              The response from the child context create call.
 * \param resp_size         The size of the response.
 */
void ups_dispatch_dataservice_response_block_meta_read(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size)
{
    dataservice_response_block_get_t dresp;

    /* decode the response. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_response_block_get(
            resp, resp_size, &dresp))
    {
        /* TODO - handle failure. */
        return;
    }

    /* get the connection associated with this child id. */
    unauthorized_protocol_connection_t* conn =
        svc->dataservice_child_map[dresp.hdr.offset];
    if (NULL == conn)
    {
        /* TODO - how do we handle a failure here? */
        goto cleanup_dresp;
    }


    /* dispatch based on the connection request. */
    switch (conn->request_id)
    {
        case UNAUTH_PROTOCOL_REQ_ID_BLOCK_BY_ID_GET:
            ups_dispatch_dataservice_response_block_read(conn, &dresp);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_BLOCK_ID_GET_NEXT:
            ups_dispatch_dataservice_response_block_read_id_next(conn, &dresp);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_BLOCK_ID_GET_PREV:
            ups_dispatch_dataservice_response_block_read_id_prev(conn, &dresp);
            break;

        default:
            unauthorized_protocol_service_error_response(
                conn, conn->request_id,
                AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_RESPONSE,
                conn->current_request_offset, true);
            break;
    }

    /* success. */

cleanup_dresp:
    dispose((disposable_t*)&dresp);
}
