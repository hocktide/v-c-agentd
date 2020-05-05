/**
 * \file protocolservice/unauthorized_protocol_service_handle_request_block_id_by_height_get.c
 *
 * \brief Handle a block id by height get request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Handle a get block id by height request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_block_id_by_height_get(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size)
{
    int retval;
    uint64_t net_block_height;
    uint64_t block_height;

    /* compute the height size. */
    const size_t height_size = sizeof(block_height);

    /* verify that the size is equal to the block height. */
    if (height_size != size)
    {
        unauthorized_protocol_service_error_response(
            conn, conn->request_id,
            AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST,
            request_offset, true);
        return;
    }

    /* read the block height in network order. */
    memcpy(&net_block_height, breq, sizeof(net_block_height));

    /* decode the block height. */
    block_height = ntohll(net_block_height);

    /* scroll past this height. */
    breq += height_size;
    size -= height_size;

    /* save the request offset. */
    conn->current_request_offset = request_offset;

    /* wait on the response from the "app" (dataservice) */
    conn->state = APCS_READ_COMMAND_RESP_FROM_APP;

    /* write the request to the dataservice using our child context. */
    /* TODO - this needs to go to the application service. */
    retval =
        dataservice_api_sendreq_block_id_by_height_get(
            &conn->svc->data, conn->dataservice_child_context,
            block_height);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_error_response(
            conn, conn->request_id,
            retval,
            request_offset, true);
        goto cleanup;
    }

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        &conn->svc->data, &unauthorized_protocol_service_dataservice_write,
        &conn->svc->loop);

cleanup:
    net_block_height = 0;
    block_height = 0;
}
