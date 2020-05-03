/**
 * \file * protocolservice/unauthorized_protocol_service_handle_request_artifact_last_txn_get.c
 *
 * \brief Handle an artifact get last transaction id request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Handle an artifact get last txn id request.
 *
 * \param conn              The connection.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_artifact_last_txn_get(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size)
{
    int retval;
    uint8_t artifact_id[16];

    /* compute the id sizes. */
    const size_t id_size = sizeof(artifact_id);

    /* verify that the size is equal to the transaction id. */
    if (id_size != size)
    {
        unauthorized_protocol_service_error_response(
            conn, conn->request_id,
            AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST,
            request_offset, true);
        return;
    }

    /* decode the block_id. */
    memcpy(artifact_id, breq, sizeof(artifact_id));

    /* scroll past this id. */
    breq += id_size;
    size -= id_size;

    /* save the request offset. */
    conn->current_request_offset = request_offset;

    /* wait on the response from the "app" (dataservice) */
    conn->state = APCS_READ_COMMAND_RESP_FROM_APP;

    /* write the request to the dataservice using our child context. */
    /* TODO - this needs to go to the application service. */
    retval =
        dataservice_api_sendreq_artifact_get(
            &conn->svc->data, conn->dataservice_child_context, artifact_id);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_error_response(
            conn, conn->request_id,
            retval,
            request_offset, true);
        goto cleanup_ids;
    }

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        &conn->svc->data, &unauthorized_protocol_service_dataservice_write,
        &conn->svc->loop);

cleanup_ids:
    memset(artifact_id, 0, sizeof(artifact_id));
}
