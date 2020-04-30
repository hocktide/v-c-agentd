/**
 * \file protocolservice/ups_dispatch_dataservice_response_txn_read_block_id.c
 *
 * \brief Handle the response from the dataservice transaction read request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * Handle a transaction read block id response.
 *
 * \param conn              The peer connection context.
 * \param dresp             The decoded response.
 */
void ups_dispatch_dataservice_response_txn_read_block_id(
    unauthorized_protocol_connection_t* conn,
    const dataservice_response_canonized_transaction_get_t* dresp)
{
    /* build the payload. */
    uint32_t net_method =
        htonl(UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_ID_GET_BLOCK_ID);
    uint32_t net_status = htonl(dresp->hdr.status);
    uint32_t net_offset = htonl(conn->current_request_offset);

    /* if the API call wasn't successful, return the error payload. */
    if (AGENTD_STATUS_SUCCESS != dresp->hdr.status)
    {
        uint8_t payload[3 * sizeof(uint32_t)];
        memcpy(payload, &net_method, 4);
        memcpy(payload + 4, &net_status, 4);
        memcpy(payload + 8, &net_offset, 4);

        /* attempt to write this payload to the socket. */
        if (AGENTD_STATUS_SUCCESS !=
            ipc_write_authed_data_noblock(
                &conn->ctx, conn->server_iv, payload, sizeof(payload),
                &conn->svc->suite, &conn->shared_secret))
        {
            unauthorized_protocol_service_close_connection(conn);
            return;
        }
    }
    /* full payload. */
    else
    {
        size_t payload_size =
            /* method, status, offset */
            3 * sizeof(uint32_t)
            /* next */
            + 1 * 16;
        uint8_t* payload = (uint8_t*)malloc(payload_size);
        if (NULL == payload)
        {
            unauthorized_protocol_service_error_response(
                conn, UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_ID_GET_BLOCK_ID,
                AGENTD_ERROR_GENERAL_OUT_OF_MEMORY,
                conn->current_request_offset, true);
            return;
        }

        /* populate header info. */
        memcpy(payload, &net_method, 4);
        memcpy(payload + 4, &net_status, 4);
        memcpy(payload + 8, &net_offset, 4);

        /* populate block info. */
        memcpy(payload + 12, dresp->node.block_id, 16);

        /* attempt to write this payload to the socket. */
        int retval =
            ipc_write_authed_data_noblock(
                &conn->ctx, conn->server_iv, payload, payload_size,
                &conn->svc->suite, &conn->shared_secret);

        /* clean up payload. */
        memset(payload, 0, payload_size);
        free(payload);

        /* check status of write. */
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            unauthorized_protocol_service_close_connection(conn);
            return;
        }
    }

    /* Update the server iv on success. */
    ++conn->server_iv;

    /* evolve connection state. */
    conn->state = APCS_WRITE_COMMAND_RESP_TO_CLIENT;

    /* set the write callback. */
    ipc_set_writecb_noblock(
        &conn->ctx, &unauthorized_protocol_service_connection_write,
        &conn->svc->loop);

    /* success. */
}
