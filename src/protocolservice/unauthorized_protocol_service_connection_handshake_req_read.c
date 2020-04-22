/**
 * \file
 * protocolservice/unauthorized_protocol_service_connection_handshake_req_read.c
 *
 * \brief Read a handshake request from the client socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Attempt to read a handshake request from the client.
 *
 * \param conn      The connection from which this handshake request should be
 *                  read.
 */
void unauthorized_protocol_service_connection_handshake_req_read(
    unauthorized_protocol_connection_t* conn)
{
    void* req = NULL;
    uint32_t size = 0U;
    uint32_t request_id;
    uint32_t request_offset;
    uint32_t protocol_version;
    uint32_t crypto_suite;

    /* attempt to read the request packet. */
    int retval = ipc_read_data_noblock(&conn->ctx, &req, &size);
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
        unauthorized_protocol_service_close_connection(conn);
        return;
    }

    /* from here on, we are committed.  Don't call this callback again. */
    ipc_set_readcb_noblock(&conn->ctx, NULL, &conn->svc->loop);

    /* set up the read buffer pointer. */
    const uint8_t* breq = (const uint8_t*)req;

    /* verify that the size matches what we expect. */
    const size_t request_id_size = sizeof(request_id);
    const size_t request_offset_size = sizeof(request_offset);
    const size_t protocol_version_size = sizeof(protocol_version);
    const size_t crypto_suite_size = sizeof(crypto_suite);
    const size_t entity_uuid_size = sizeof(conn->entity_uuid);
    const size_t expected_size =
        request_id_size + request_offset_size + protocol_version_size + crypto_suite_size + entity_uuid_size + conn->client_key_nonce.size + conn->client_challenge_nonce.size;
    if (size != expected_size)
    {
        unauthorized_protocol_service_error_response(
            conn, 0, AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0, false);
        goto cleanup_data;
    }

    /* read the request ID.  It should be 0x00000000 for handshake request. */
    memcpy(&request_id, breq, request_id_size);
    breq += request_id_size;
    request_id = ntohl(request_id);
    if (UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE != request_id)
    {
        unauthorized_protocol_service_error_response(
            conn, 0, AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0, false);
        goto cleanup_data;
    }

    /* read the request offset.  It should be 0x00000000. */
    memcpy(&request_offset, breq, request_offset_size);
    breq += request_offset_size;
    request_offset = ntohl(request_offset);
    if (0x00000000 != request_offset)
    {
        unauthorized_protocol_service_error_response(
            conn, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
            AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0, false);
        goto cleanup_data;
    }

    /* read the protocol verson.  It should be 0x00000001. */
    memcpy(&protocol_version, breq, protocol_version_size);
    breq += protocol_version_size;
    protocol_version = ntohl(protocol_version);
    if (0x00000001 != protocol_version)
    {
        unauthorized_protocol_service_error_response(
            conn, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
            AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0, false);
        goto cleanup_data;
    }

    /* read the crypto suite verson.  It should be VCCRYPT_SUITE_VELO_V1. */
    memcpy(&crypto_suite, breq, crypto_suite_size);
    breq += crypto_suite_size;
    crypto_suite = ntohl(crypto_suite);
    if (VCCRYPT_SUITE_VELO_V1 != crypto_suite)
    {
        unauthorized_protocol_service_error_response(
            conn, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
            AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0, false);
        goto cleanup_data;
    }

    /* read the entity uuid. */
    memcpy(conn->entity_uuid, breq, entity_uuid_size);
    breq += entity_uuid_size;

    /* read the client key nonce. */
    memcpy(conn->client_key_nonce.data, breq, conn->client_key_nonce.size);
    breq += conn->client_key_nonce.size;

    /* read the client challenge nonce. */
    memcpy(
        conn->client_challenge_nonce.data, breq,
        conn->client_challenge_nonce.size);
    breq += conn->client_challenge_nonce.size;

    /* Verify that this is a valid entity. */
    if (AGENTD_STATUS_SUCCESS !=
        unauthorized_protocol_service_get_entity_key(conn))
    {
        unauthorized_protocol_service_error_response(
            conn, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
            AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED, 0, false);
        goto cleanup_data;
    }

    /* write an entropy request to the random service to gather entropy for this
     * handshake. */
    if (AGENTD_STATUS_SUCCESS !=
        unauthorized_protocol_service_write_entropy_request(conn))
    {
        unauthorized_protocol_service_error_response(
            conn, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
            AGENTD_ERROR_PROTOCOLSERVICE_PRNG_REQUEST_FAILURE, 0, false);
        goto cleanup_data;
    }

    /* success */

cleanup_data:
    memset(req, 0, size);
    free(req);
}
