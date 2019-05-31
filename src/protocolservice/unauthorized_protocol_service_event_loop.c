/**
 * \file protocolservice/unauthorized_protocol_service_event_loop.c
 *
 * \brief The event loop for the unauthorized protocol service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <agentd/protocolservice.h>
#include <agentd/randomservice.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <vccrypt/compare.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/* forward decls */
static void unauthorized_protocol_service_ipc_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void unauthorized_protocol_service_close_connection(
    unauthorized_protocol_connection_t* conn);
static void unauthorized_protocol_service_protosock_cb_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void unauthorized_protocol_service_handshake_req_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void unauthorized_protocol_service_random_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void unauthorized_protocol_service_random_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static int unauthorized_protocol_service_get_entity_key(
    unauthorized_protocol_connection_t* conn);
static int unauthorized_protocol_service_write_entropy_request(
    unauthorized_protocol_connection_t* conn);
static int unauthorized_protocol_service_write_handshake_request_response(
    unauthorized_protocol_connection_t* conn);
static void unauthorized_protocol_service_error_response(
    unauthorized_protocol_connection_t* conn, int request_id, int status,
    uint32_t offset);

/**
 * \brief Event loop for the unauthorized protocol service.  This is the entry
 * point for the protocol service.  It handles the details of reacting to events
 * sent over the protocol service socket.
 *
 * \param randomsock    The socket to the RNG service.
 * \param protosock     The protocol service socket.  The protocol service
 *                      listens for connections on this socket.
 * \param logsock       The logging service socket.  The protocol service logs
 *                      on this socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *          attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if adding
 *            the protocol service socket to the event loop failed.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if running
 *            the protocol service event loop failed.
 */
int unauthorized_protocol_service_event_loop(
    int randomsock, int protosock, int UNUSED(logsock))
{
    int retval = 0;
    unauthorized_protocol_service_instance_t inst;

    /* parameter sanity checking. */
    MODEL_ASSERT(protosock >= 0);
    MODEL_ASSERT(logsock >= 0);

    /* initialize this instance. */
    /* TODO - get the number of connections from config. */
    retval =
        unauthorized_protocol_service_instance_init(
            &inst, randomsock, protosock, 50);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* set the read callback for the protocol socket. */
    ipc_set_readcb_noblock(
        &inst.proto, &unauthorized_protocol_service_ipc_read, NULL);

    /* add the protocol socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(&inst.loop, &inst.proto))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_ADD_FAILURE;
        goto cleanup_inst;
    }

    /* set the read callback for the random socket. */
    ipc_set_readcb_noblock(
        &inst.random, &unauthorized_protocol_service_random_read, NULL);

    /* add the random socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(&inst.loop, &inst.random))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_ADD_FAILURE;
        goto cleanup_inst;
    }

    /* run the ipc event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_run(&inst.loop))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_RUN_FAILURE;
        goto cleanup_inst;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_inst:
    dispose((disposable_t*)&inst);

done:
    return retval;
}

/**
 * \brief Handle read events on the protocol socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this proto socket.
 */
static void unauthorized_protocol_service_ipc_read(
    ipc_socket_context_t* ctx, int UNUSED(event_flags),
    void* user_context)
{
    int recvsock;

    /* TODO - This should be non-blocking on the listener and protocol side. */

    /* get the instance from the user context. */
    unauthorized_protocol_service_instance_t* inst =
        (unauthorized_protocol_service_instance_t*)user_context;

    /* attempt to receive a socket from the listen service. */
    int retval = ipc_receivesocket_noblock(ctx, &recvsock);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        return;
    }

    /* Try to get a connection for this socket. */
    unauthorized_protocol_connection_t* conn = inst->free_connection_head;
    if (NULL == conn)
    {
        close(recvsock);
        return;
    }

    /* Remove this connection from the free list. */
    unauthorized_protocol_connection_remove(&inst->free_connection_head, conn);

    /* initialize this connection with the socket. */
    if (AGENTD_STATUS_SUCCESS !=
        unauthorized_protocol_connection_init(conn, recvsock, inst))
    {
        unauthorized_protocol_connection_push_front(
            &inst->free_connection_head, conn);
        close(recvsock);
        return;
    }

    /* add the socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(&inst->loop, &conn->ctx))
    {
        dispose((disposable_t*)conn);
        unauthorized_protocol_connection_push_front(
            &inst->free_connection_head, conn);
        close(recvsock);
        return;
    }

    /* set the read callback for this connection. */
    ipc_set_readcb_noblock(
        &conn->ctx, &unauthorized_protocol_service_handshake_req_read,
        &conn->svc->loop);

    /* this is now a used connection. */
    unauthorized_protocol_connection_push_front(
        &inst->used_connection_head, conn);
}

/**
 * \brief Read the initial handshake request.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this proto socket.
 */
static void unauthorized_protocol_service_handshake_req_read(
    ipc_socket_context_t* ctx, int UNUSED(event_flags),
    void* user_context)
{
    /* get the instance from the user context. */
    unauthorized_protocol_connection_t* conn =
        (unauthorized_protocol_connection_t*)user_context;

    void* req = NULL;
    uint32_t size = 0U;
    uint32_t request_id;
    uint32_t request_offset;
    uint32_t protocol_version;
    uint32_t crypto_suite;

    /* attempt to read the request packet. */
    int retval = ipc_read_data_noblock(ctx, &req, &size);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK == retval)
    {
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
            conn, 0, AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0);
        goto cleanup_data;
    }

    /* read the request ID.  It should be 0x00000000 for handshake request. */
    memcpy(&request_id, breq, request_id_size);
    breq += request_id_size;
    request_id = ntohl(request_id);
    if (UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE != request_id)
    {
        unauthorized_protocol_service_error_response(
            conn, 0, AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0);
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
            AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0);
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
            AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0);
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
            AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, 0);
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
            AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED, 0);
        goto cleanup_data;
    }

    /* write an entropy request to the random service to gather entropy for this
     * handshake. */
    if (AGENTD_STATUS_SUCCESS !=
        unauthorized_protocol_service_write_entropy_request(conn))
    {
        unauthorized_protocol_service_error_response(
            conn, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE,
            AGENTD_ERROR_PROTOCOLSERVICE_PRNG_REQUEST_FAILURE, 0);
        goto cleanup_data;
    }

    /* success */

cleanup_data:
    memset(req, 0, size);
    free(req);
}

/**
 * \brief Close a connection, returning it to the free connection pool.
 *
 * \param conn          The connection to close.
 */
static void unauthorized_protocol_service_close_connection(
    unauthorized_protocol_connection_t* conn)
{
    unauthorized_protocol_service_instance_t* svc =
        (unauthorized_protocol_service_instance_t*)conn->svc;

    ipc_event_loop_remove(&svc->loop, &conn->ctx);
    unauthorized_protocol_connection_remove(
        &svc->used_connection_head, conn);
    dispose((disposable_t*)conn);
    unauthorized_protocol_connection_push_front(
        &svc->free_connection_head, conn);
}

/**
 * \brief Get the entity key associated with the data read during a handshake
 * request.
 *
 * \param conn          The connection for which the entity key should be
 *                      resolved.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero return code on failure.
 */
static int unauthorized_protocol_service_get_entity_key(
    unauthorized_protocol_connection_t* conn)
{
    /* verify that the entity id is authorized. */
    /* TODO - this should perform a database lookup. */
    if (0 != crypto_memcmp(conn->entity_uuid, conn->svc->authorized_entity_id, 16))
    {
        return 1;
    }

    /* the entity id is valid, so copy the entity public key. */
    memcpy(
        conn->entity_public_key.data, conn->svc->authorized_entity_pubkey.data,
        conn->entity_public_key.size);

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Write a request to the random service to gather entropy.
 *
 * \param conn          The connection for which the request is written.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero return code on failure.
 */
static int unauthorized_protocol_service_write_entropy_request(
    unauthorized_protocol_connection_t* conn)
{
    /* TODO - replace with random API method. */
    uint32_t payload[3] = {
        htonl(RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES),
        htonl(conn - conn->svc->connections),
        htonl(conn->server_key_nonce.size + conn->server_challenge_nonce.size)
    };

    /* attempt to write the request payload to the random socket. */
    int retval =
        ipc_write_data_noblock(&conn->svc->random, payload, sizeof(payload));
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        return AGENTD_ERROR_PROTOCOLSERVICE_PRNG_REQUEST_FAILURE;
    }

    /* set state to gathering entropy. */
    conn->state = UPCS_HANDSHAKE_GATHER_ENTROPY;

    /* set the write callback for the random socket. */
    ipc_set_writecb_noblock(
        &conn->svc->random, &unauthorized_protocol_service_random_write,
        &conn->svc->loop);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Compute and write the handshake response for the handshake request.
 *
 * \param conn          The connection for which the response should be written.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero return code on failure.
 */
static int unauthorized_protocol_service_write_handshake_request_response(
    unauthorized_protocol_connection_t* conn)
{
    int retval = AGENTD_STATUS_SUCCESS;

    /* | Handshake request response packet.                                 | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATA                                                | SIZE         | */
    /* | --------------------------------------------------- | ------------ | */
    /* | UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE           |   4 bytes    | */
    /* | offset                                              |   4 bytes    | */
    /* | status                                              |   4 bytes    | */
    /* | record:                                             | 152 bytes    | */
    /* |    protocol_version                                 |   4 bytes    | */
    /* |    crypto_suite                                     |   4 bytes    | */
    /* |    agent_id                                         |  16 bytes    | */
    /* |    agent public key                                 |  32 bytes    | */
    /* |    server key nonce                                 |  32 bytes    | */
    /* |    server challenge nonce                           |  32 bytes    | */
    /* |    server_cr_hmac                                   |  32 bytes    | */
    /* | --------------------------------------------------- | ------------ | */

    /* create key agreement instance. */
    vccrypt_key_agreement_context_t agreement;
    retval =
        vccrypt_suite_cipher_key_agreement_init(
            &conn->svc->suite, &agreement);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* Derive the shared secret using the key nonces. */
    retval =
        vccrypt_key_agreement_short_term_secret_create(
            &agreement, &conn->svc->agent_privkey, &conn->entity_public_key,
            &conn->server_key_nonce, &conn->client_key_nonce,
            &conn->shared_secret);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agreement;
    }

    /* Compute the response packet payload size. */
    uint32_t request_id = htonl(UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE);
    uint32_t offset = htonl(0x00U);
    uint32_t protocol_version = htonl(0x00000001);
    uint32_t crypto_suite = htonl(VCCRYPT_SUITE_VELO_V1);
    int32_t status = htonl(AGENTD_STATUS_SUCCESS);
    size_t payload_size =
        sizeof(request_id) + sizeof(offset) + sizeof(status) + sizeof(protocol_version) + sizeof(crypto_suite) + 16 /* agent id */
        + conn->svc->agent_pubkey.size + conn->server_key_nonce.size + conn->server_challenge_nonce.size + conn->svc->short_hmac.mac_size;

    /* Create the response packet payload buffer. */
    vccrypt_buffer_t payload;
    retval =
        vccrypt_buffer_init(&payload, &conn->svc->alloc_opts, payload_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agreement;
    }

    /* convenience pointer for working with this buffer. */
    uint8_t* buf_start = (uint8_t*)payload.data;
    uint8_t* pbuf = (uint8_t*)payload.data;

    /* write the payload values to this buffer. */
    memcpy(pbuf, &request_id, sizeof(request_id));
    pbuf += sizeof(request_id);
    memcpy(pbuf, &offset, sizeof(offset));
    pbuf += sizeof(offset);
    memcpy(pbuf, &status, sizeof(status));
    pbuf += sizeof(status);
    memcpy(pbuf, &protocol_version, sizeof(protocol_version));
    pbuf += sizeof(protocol_version);
    memcpy(pbuf, &crypto_suite, sizeof(crypto_suite));
    pbuf += sizeof(crypto_suite);
    memcpy(pbuf, conn->svc->agent_id, 16);
    pbuf += 16;
    memcpy(pbuf, conn->svc->agent_pubkey.data, conn->svc->agent_pubkey.size);
    pbuf += conn->svc->agent_pubkey.size;
    memcpy(pbuf, conn->server_key_nonce.data, conn->server_key_nonce.size);
    pbuf += conn->server_key_nonce.size;
    memcpy(
        pbuf, conn->server_challenge_nonce.data,
        conn->server_challenge_nonce.size);
    pbuf += conn->server_challenge_nonce.size;

    /* create an HMAC instance. */
    vccrypt_mac_context_t mac;
    retval =
        vccrypt_mac_init(&conn->svc->short_hmac, &mac, &conn->shared_secret);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_payload;
    }

    /* digest the response packet.*/
    retval = vccrypt_mac_digest(&mac, buf_start, pbuf - buf_start);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* add the client challenge nonce to the response packet. */
    retval =
        vccrypt_mac_digest(
            &mac, conn->client_challenge_nonce.data,
            conn->client_challenge_nonce.size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* create buffer for holding mac output. */
    vccrypt_buffer_t mac_buffer;
    retval =
        vccrypt_buffer_init(
            &mac_buffer, &conn->svc->alloc_opts,
            conn->svc->short_hmac.mac_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* finalize the mac. */
    retval = vccrypt_mac_finalize(&mac, &mac_buffer);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac_buffer;
    }

    /* copy the hmac to the payload. */
    memcpy(pbuf, mac_buffer.data, mac_buffer.size);
    pbuf += mac_buffer.size;

    /* write packet to connection and set write callback. */
    retval = ipc_write_data_noblock(&conn->ctx, payload.data, payload.size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac_buffer;
    }

    /* set state to UPCS_WRITE_HANDSHAKE_RESP_TO_CLIENT */
    conn->state = UPCS_WRITE_HANDSHAKE_RESP_TO_CLIENT;

    /* set the write callback. */
    ipc_set_writecb_noblock(
        &conn->ctx, &unauthorized_protocol_service_protosock_cb_write,
        &conn->svc->loop);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_mac_buffer:
    dispose((disposable_t*)&mac_buffer);

cleanup_mac:
    dispose((disposable_t*)&mac);

cleanup_payload:
    dispose((disposable_t*)&payload);

cleanup_agreement:
    dispose((disposable_t*)&agreement);

done:
    return retval;
}

/**
 * \brief Write an error response to the socket and set the connection state to
 * unauthorized.
 *
 * This method writes an error response to the socket and sets up the state
 * machine to close the connection after the error response is written.
 *
 * \param conn          The connection to which the error response is written.
 * \param request_id    The id of the request that caused the error.
 * \param status        The status code of the error.
 * \param offset        The request offset that caused the error.
 */
static void unauthorized_protocol_service_error_response(
    unauthorized_protocol_connection_t* conn, int request_id, int status,
    uint32_t offset)
{
    int retval = AGENTD_STATUS_SUCCESS;

    uint32_t payload[3] = { htonl(request_id), htonl(status), htonl(offset) };

    /* attempt to write the response payload to the socket. */
    retval = ipc_write_data_noblock(&conn->ctx, payload, sizeof(payload));
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_close_connection(conn);
        return;
    }

    /* set state to unauthorized so we disconnect after writing the error. */
    conn->state = UPCS_UNAUTHORIZED;

    /* set the write callback for the protocol socket. */
    ipc_set_writecb_noblock(
        &conn->ctx, &unauthorized_protocol_service_protosock_cb_write,
        &conn->svc->loop);
}

/**
 * \brief The write callback for managing writes to the client connection and
 * for advancing the state machine after the write is completed.
 *
 * \param ctx           The socket context for this write callback.
 * \param event_flags   The event flags that led to this callback being called.
 * \param user_context  The user context for this callback (expected: a protocol
 *                      connection).
 */
static void unauthorized_protocol_service_protosock_cb_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    /* get the instance from the user context. */
    unauthorized_protocol_connection_t* conn =
        (unauthorized_protocol_connection_t*)user_context;

    /* first, see if we still need to write data. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        /* attempt to write data. */
        int bytes_written = ipc_socket_write_from_buffer(ctx);

        /* was the socket closed, or was there an error? */
        if (bytes_written == 0 || (bytes_written < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)))
        {
            unauthorized_protocol_service_close_connection(conn);
            return;
        }
    }
    else
    {
        /* should we change state? */
        switch (conn->state)
        {
            case UPCS_UNAUTHORIZED:
                unauthorized_protocol_service_close_connection(conn);
                return;

            /* unexpected state.  Close connection. */
            default:
                unauthorized_protocol_service_close_connection(conn);
                return;
        }
    }
}

/**
 * \brief Write data to the random service socket.
 */
static void unauthorized_protocol_service_random_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    /* get the instance from the user context. */
    unauthorized_protocol_service_instance_t* svc =
        (unauthorized_protocol_service_instance_t*)user_context;

    /* first, see if we still need to write data. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        /* attempt to write data. */
        int bytes_written = ipc_socket_write_from_buffer(ctx);

        /* was the socket closed, or was there an error? */
        if (bytes_written == 0 || (bytes_written < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)))
        {
            /* TODO - shut down service. This shouldn't happen. */
            return;
        }
    }
    else
    {
        ipc_set_writecb_noblock(&svc->random, NULL, &svc->loop);
    }
}

/**
 * \brief Write data to the random service socket.
 */
static void unauthorized_protocol_service_random_read(
    ipc_socket_context_t* UNUSED(ctx), int UNUSED(event_flags),
    void* user_context)
{
    void* resp = NULL;
    uint8_t* bresp = NULL;
    uint32_t resp_size = 0U;
    uint32_t size = 0U;

    uint32_t request_id;
    uint32_t request_offset;
    uint32_t status;

    /* get the instance from the user context. */
    unauthorized_protocol_service_instance_t* svc =
        (unauthorized_protocol_service_instance_t*)user_context;

    /* attempt to read a response packet. */
    int retval = ipc_read_data_noblock(&svc->random, &resp, &resp_size);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK == retval)
    {
        return;
    }
    /* handle general failures from the random service socket read. */
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        /* TODO - shut down service. */
        return;
    }

    /* set up buffer pointer. */
    bresp = resp;
    size = resp_size;

    /* verify size */
    if (size < 3 * sizeof(uint32_t))
    {
        /* TODO - shut down sevice. */
        goto cleanup_resp;
    }

    /* decode the method and verify. */
    memcpy(&request_id, bresp, sizeof(request_id));
    bresp += sizeof(request_id);
    size -= sizeof(request_id);
    request_id = ntohl(request_id);
    if (RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES != request_id)
    {
        /* TODO - shut down service. */
        goto cleanup_resp;
    }

    /* decode the request offset and verify. */
    memcpy(&request_offset, bresp, sizeof(request_offset));
    bresp += sizeof(request_offset);
    size -= sizeof(request_offset);
    request_offset = ntohl(request_offset);
    if (request_offset >= svc->num_connections)
    {
        /* TODO - shut down service. */
        goto cleanup_resp;
    }

    /* get the connection and verify that it is in the right state. */
    unauthorized_protocol_connection_t* conn =
        svc->connections + request_offset;
    if (UPCS_HANDSHAKE_GATHER_ENTROPY != conn->state)
    {
        /* has the connection already been closed? */
        if (UPCS_HANDSHAKE_GATHER_ENTROPY_CLOSED == conn->state)
        {
            goto cleanup_resp;
        }

        /* force the connection closed. */
        unauthorized_protocol_service_close_connection(conn);
        goto cleanup_resp;
    }

    /* get the status and verify that this request was successful. */
    memcpy(&status, bresp, sizeof(status));
    bresp += sizeof(status);
    size -= sizeof(status);
    status = ntohl(status);
    if (AGENTD_STATUS_SUCCESS != status)
    {
        /* log the error to the socket. */
        unauthorized_protocol_service_error_response(
            conn, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE, status, 0);
        goto cleanup_resp;
    }

    /* copy payload data. */
    memcpy(conn->server_key_nonce.data, bresp, conn->server_key_nonce.size);
    bresp += conn->server_key_nonce.size;
    size -= conn->server_key_nonce.size;
    memcpy(
        conn->server_challenge_nonce.data, bresp,
        conn->server_challenge_nonce.size);

    /* write the handshake request response and walk state / callbacks. */
    if (AGENTD_STATUS_SUCCESS !=
        unauthorized_protocol_service_write_handshake_request_response(
            conn))
    {
        unauthorized_protocol_service_close_connection(conn);
        goto cleanup_resp;
    }

    /* success. */

cleanup_resp:
    memset(resp, 0, resp_size);
    free(resp);
}
