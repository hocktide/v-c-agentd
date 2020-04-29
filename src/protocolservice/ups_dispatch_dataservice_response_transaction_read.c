/**
 * \file protocolservice/ups_dispatch_dataservice_response_transaction_read.c
 *
 * \brief Handle the response from the dataservice transaction read request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * Handle a transaction read response.
 *
 * \param svc               The protocol service instance.
 * \param resp              The response from the child context create call.
 * \param resp_size         The size of the response.
 */
void ups_dispatch_dataservice_response_transaction_read(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size)
{
    dataservice_response_canonized_transaction_get_t dresp;

    /* decode the response. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_response_canonized_transaction_get(
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

    /* build the payload. */
    uint32_t net_method = htonl(UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_BY_ID_GET);
    uint32_t net_status = htonl(dresp.hdr.status);
    uint32_t net_offset = htonl(conn->current_request_offset);

    /* if the API call wasn't successful, return the error payload. */
    if (AGENTD_STATUS_SUCCESS != dresp.hdr.status)
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
            goto cleanup_dresp;
        }
    }
    /* full payload. */
    else
    {
        size_t payload_size =
            /* method, status, offset */
            3 * sizeof(uint32_t)
            /* key, prev, next, artifact_id, block_id */
            + 5 * 16
            /* net_txn_cert_size */
            + 1 * 8
            /* net_txn_state */
            + 1 * 4
            /* block cert. */
            + dresp.data_size;
        uint8_t* payload = (uint8_t*)malloc(payload_size);
        if (NULL == payload)
        {
            unauthorized_protocol_service_error_response(
                conn, UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_BY_ID_GET,
                AGENTD_ERROR_GENERAL_OUT_OF_MEMORY,
                conn->current_request_offset, true);
            goto cleanup_dresp;
        }

        /* populate header info. */
        memcpy(payload, &net_method, 4);
        memcpy(payload + 4, &net_status, 4);
        memcpy(payload + 8, &net_offset, 4);

        /* populate block info. */
        memcpy(payload + 12, dresp.node.key, 16);
        memcpy(payload + 28, dresp.node.prev, 16);
        memcpy(payload + 44, dresp.node.next, 16);
        memcpy(payload + 60, dresp.node.artifact_id, 16);
        memcpy(payload + 76, &dresp.node.block_id, 16);
        memcpy(payload + 92, &dresp.node.net_txn_cert_size, 8);
        memcpy(payload + 100, &dresp.node.net_txn_state, 4);

        /* populate certificate. */
        memcpy(payload + 104, dresp.data, dresp.data_size);

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
            goto cleanup_dresp;
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

cleanup_dresp:
    dispose((disposable_t*)&dresp);
}
