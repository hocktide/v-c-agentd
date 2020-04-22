/**
 * \file protocolservice/unauthorized_protocol_service_command_read.c
 *
 * \brief Read a command from an authenticated connection.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Attempt to read a command from the client.
 *
 * \param conn      The connection from which this command should be read.
 */
void unauthorized_protocol_service_command_read(
    unauthorized_protocol_connection_t* conn)
{
    void* req = NULL;
    uint32_t size = 0U;
    uint32_t request_id;
    uint32_t request_offset;

    /* attempt to read the command packet. */
    int retval =
        ipc_read_authed_data_noblock(
            &conn->ctx, conn->client_iv, &req, &size, &conn->svc->suite,
            &conn->shared_secret);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK == retval)
    {
        ipc_set_readcb_noblock(
            &conn->ctx, &unauthorized_protocol_service_connection_read,
            &conn->svc->loop);
        return;
    }
    if (AGENTD_ERROR_IPC_EVBUFFER_READ_FAILURE == retval || AGENTD_ERROR_IPC_EVBUFFER_EOF == retval)
    {
        unauthorized_protocol_service_close_connection(conn);
        return;
    }
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_error_response(
            conn, 0, AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0, true);
        return;
    }

    /* from here on, we are committed.  Don't call this callback again. */
    ++conn->client_iv;
    ipc_set_readcb_noblock(&conn->ctx, NULL, &conn->svc->loop);

    /* set up the read buffer pointer. */
    const uint8_t* breq = (const uint8_t*)req;

    /* verify that the size matches what we expect. */
    const size_t request_id_size = sizeof(request_id);
    const size_t request_offset_size = sizeof(request_offset);
    const size_t expected_size =
        request_id_size + request_offset_size;
    if (size < expected_size)
    {
        unauthorized_protocol_service_error_response(
            conn, 0, AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0, false);
        goto cleanup_data;
    }

    /* read the request ID. */
    memcpy(&request_id, breq, request_id_size);
    breq += request_id_size;
    request_id = ntohl(request_id);

    /* read the request offset. */
    memcpy(&request_offset, breq, request_offset_size);
    breq += request_offset_size;
    request_offset = ntohl(request_offset);

    /* decode and dispatch this request. */
    unauthorized_protocol_service_decode_and_dispatch(
        conn, request_id, request_offset, breq, size - expected_size);

    /* fall-through to clean up data. */
cleanup_data:
    memset(req, 0, size);
    free(req);
}
