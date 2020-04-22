/**
 * \file protocolservice/unauthorized_protocol_service_handle_request_transaction_submit.c
 *
 * \brief Handle a submit transaction request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

#define MAX_TRANSACTION_CERTIFICATE_SIZE 32767

/**
 * \brief Handle a transaction submit request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_transaction_submit(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size)
{
    int retval;
    uint8_t txn_id[16];
    uint8_t artifact_id[16];

    /* compute the id sizes. */
    const size_t id_size = sizeof(txn_id) + sizeof(artifact_id);

    /* verify that the size is at least large enough for the UUIDs. */
    if (size < id_size)
    {
        unauthorized_protocol_service_error_response(
            conn, UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_SUBMIT,
            AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST,
            request_offset, true);
        return;
    }

    /* decode the txn_id. */
    memcpy(txn_id, breq, sizeof(txn_id));

    /* decode the artifact_id. */
    memcpy(artifact_id, breq + sizeof(txn_id), sizeof(artifact_id));

    /* scroll past these ids. */
    breq += id_size;
    size -= id_size;

    /* if the transaction cert size is too large, report an error. */
    if (size > MAX_TRANSACTION_CERTIFICATE_SIZE)
    {
        unauthorized_protocol_service_error_response(
            conn, UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_SUBMIT,
            AGENTD_ERROR_PROTOCOLSERVICE_TRANSACTION_VERIFICATION,
            request_offset, true);
        goto cleanup_ids;
    }

    /* save the request offset. */
    conn->current_request_offset = request_offset;

    /* wait on the response from the "app" (dataservice) */
    conn->state = APCS_READ_COMMAND_RESP_FROM_APP;

    /* write the request to the dataservice using our child context. */
    /* TODO - this needs to go to the application service. */
    retval =
        dataservice_api_sendreq_transaction_submit(
            &conn->svc->data, conn->dataservice_child_context, txn_id,
            artifact_id, breq, size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_error_response(
            conn, UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_SUBMIT,
            retval,
            request_offset, true);
        goto cleanup_ids;
    }

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        &conn->svc->data, &unauthorized_protocol_service_dataservice_write,
        &conn->svc->loop);

cleanup_ids:
    memset(txn_id, 0, sizeof(txn_id));
    memset(artifact_id, 0, sizeof(artifact_id));
}
