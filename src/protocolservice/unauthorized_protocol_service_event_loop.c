/**
 * \file protocolservice/unauthorized_protocol_service_event_loop.c
 *
 * \brief The event loop for the unauthorized protocol service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
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
static void unauthorized_protocol_service_connection_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void unauthorized_protocol_service_connection_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void unauthorized_protocol_service_connection_handshake_req_read(
    unauthorized_protocol_connection_t* conn);
static void unauthorized_protocol_service_connection_handshake_ack_read(
    unauthorized_protocol_connection_t* conn);
static void unauthorized_protocol_service_command_read(
    unauthorized_protocol_connection_t* conn);
static void unauthorized_protocol_service_decode_and_dispatch(
    unauthorized_protocol_connection_t* conn, uint32_t request_id,
    uint32_t request_offset, const uint8_t* breq, size_t size);
static void unauthorized_protocol_service_handle_request_latest_block_get(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size);
static void unauthorized_protocol_service_random_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void unauthorized_protocol_service_random_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void unauthorized_protocol_service_dataservice_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void unauthorized_protocol_service_dataservice_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static int unauthorized_protocol_service_get_entity_key(
    unauthorized_protocol_connection_t* conn);
static int unauthorized_protocol_service_write_entropy_request(
    unauthorized_protocol_connection_t* conn);
static int unauthorized_protocol_service_write_handshake_request_response(
    unauthorized_protocol_connection_t* conn);
static void unauthorized_protocol_service_error_response(
    unauthorized_protocol_connection_t* conn, int request_id, int status,
    uint32_t offset, bool encrypted);
static void unauthorized_protocol_service_dataservice_request_child_context(
    unauthorized_protocol_connection_t* conn);
static void ups_dispatch_dataservice_response_child_context_create(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size);
static void ups_dispatch_dataservice_response_child_context_close(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size);
static void ups_dispatch_dataservice_response_block_id_latest_read(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size);

/**
 * \brief Event loop for the unauthorized protocol service.  This is the entry
 * point for the protocol service.  It handles the details of reacting to events
 * sent over the protocol service socket.
 *
 * \param randomsock    The socket to the RNG service.
 * \param protosock     The protocol service socket.  The protocol service
 *                      listens for connections on this socket.
 * \param datasock      The data service socket.  The protocol service
 *                      communicates with the dataservice using this socket.
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
    int randomsock, int protosock, int datasock, int UNUSED(logsock))
{
    int retval = 0;
    unauthorized_protocol_service_instance_t inst;

    /* parameter sanity checking. */
    MODEL_ASSERT(protosock >= 0);
    MODEL_ASSERT(datasock >= 0);
    MODEL_ASSERT(logsock >= 0);

    /* initialize this instance. */
    /* TODO - get the number of connections from config. */
    retval =
        unauthorized_protocol_service_instance_init(
            &inst, randomsock, datasock, protosock, 50);
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

    /* set the read callback for the dataservice socket. */
    ipc_set_readcb_noblock(
        &inst.data, &unauthorized_protocol_service_dataservice_read, NULL);

    /* add the data socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(&inst.loop, &inst.data))
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
        &conn->ctx, &unauthorized_protocol_service_connection_read,
        &conn->svc->loop);

    /* this is now a used connection. */
    unauthorized_protocol_connection_push_front(
        &inst->used_connection_head, conn);
}

/**
 * \brief Read data from the connection
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this proto socket.
 */
static void unauthorized_protocol_service_connection_read(
    ipc_socket_context_t* UNUSED(ctx), int UNUSED(event_flags),
    void* user_context)
{
    /* get the instance from the user context. */
    unauthorized_protocol_connection_t* conn =
        (unauthorized_protocol_connection_t*)user_context;

    /* dispatch based on the current connection state. */
    switch (conn->state)
    {
        /* we expect to read a handshake request from the client. */
        case UPCS_READ_HANDSHAKE_REQ_FROM_CLIENT:
            unauthorized_protocol_service_connection_handshake_req_read(conn);
            break;

        /* we expect to read a handshake acknowledgement from the client. */
        case UPCS_READ_HANDSHAKE_ACK_FROM_CLIENT:
            unauthorized_protocol_service_connection_handshake_ack_read(conn);
            break;

        /* we expect to read a command request from the client. */
        case APCS_READ_COMMAND_REQ_FROM_CLIENT:
            unauthorized_protocol_service_command_read(conn);
            break;

        default:
            break;
    }
}

/**
 * \brief Attempt to read a handshake request from the client.
 *
 * \param conn      The connection from which this handshake request should be
 *                  read.
 */
static void unauthorized_protocol_service_connection_handshake_req_read(
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
        return;
    }
    if (AGENTD_ERROR_IPC_EVBUFFER_READ_FAILURE == retval)
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

/**
 * \brief Attempt to the client challenge response acknowledgement.
 *
 * \param conn      The connection from which this ack should be read.
 */
static void unauthorized_protocol_service_connection_handshake_ack_read(
    unauthorized_protocol_connection_t* conn)
{
    void* req = NULL;
    uint32_t size = 0U;

    /* attempt to read the ack packet. */
    int retval =
        ipc_read_authed_data_noblock(
            &conn->ctx, conn->client_iv, &req, &size, &conn->svc->suite,
            &conn->shared_secret);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK == retval)
    {
        return;
    }
    if (AGENTD_ERROR_IPC_EVBUFFER_READ_FAILURE == retval)
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

    /* Build the ack payload. */
    uint32_t payload[3] = {
        htonl(UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_ACKNOWLEDGE),
        htonl(AGENTD_STATUS_SUCCESS),
        htonl(0)
    };

    /* attempt to write this payload to the socket. */
    retval =
        ipc_write_authed_data_noblock(
            &conn->ctx, conn->server_iv, payload, sizeof(payload),
            &conn->svc->suite, &conn->shared_secret);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_close_connection(conn);
        goto cleanup_data;
    }

    /* Update the server iv on success. */
    ++conn->server_iv;

    /* set the updated connection state. */
    conn->state = UPCS_WRITE_HANDSHAKE_ACK_TO_CLIENT;

    /* set the write callback. */
    ipc_set_writecb_noblock(
        &conn->ctx, &unauthorized_protocol_service_connection_write,
        &conn->svc->loop);

    /* success. */

cleanup_data:
    memset(req, 0, size);
    free(req);
}

/**
 * \brief Attempt to read a command from the client.
 *
 * \param conn      The connection from which this command should be read.
 */
static void unauthorized_protocol_service_command_read(
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
        return;
    }
    if (AGENTD_ERROR_IPC_EVBUFFER_READ_FAILURE == retval)
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

/**
 * \brief Decode and dispatch a request from the client.
 *
 * \param conn              The connection to close.
 * \param request_id        The request ID to decode.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
static void unauthorized_protocol_service_decode_and_dispatch(
    unauthorized_protocol_connection_t* conn, uint32_t request_id,
    uint32_t request_offset, const uint8_t* breq, size_t size)
{
    /* decode the request id */
    switch (request_id)
    {
        case UNAUTH_PROTOCOL_REQ_ID_LATEST_BLOCK_GET:
            unauthorized_protocol_service_handle_request_latest_block_get(
                conn, request_offset, breq, size);
            break;

        case UNAUTH_PROTOCOL_REQ_ID_CLOSE:
            unauthorized_protocol_service_error_response(
                conn, request_id, AGENTD_STATUS_SUCCESS, request_offset, true);
            break;

        /* TODO - replace with valid error code. */
        default:
            unauthorized_protocol_service_error_response(
                conn, request_id, 8675309, request_offset, true);
    }
}

/**
 * \brief Handle a latest_block_get request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
static void unauthorized_protocol_service_handle_request_latest_block_get(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* UNUSED(breq), size_t UNUSED(size))
{
    /* save the request offset. */
    conn->current_request_offset = request_offset;

    /* wait on the response from the "app" (dataservice) */
    conn->state = APCS_READ_COMMAND_RESP_FROM_APP;

    /* write the request to the dataservice using our child context. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_api_sendreq_latest_block_id_get(
            &conn->svc->data, conn->dataservice_child_context))
    {
        /* TODO - handle error. */
        return;
    }

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        &conn->svc->data, &unauthorized_protocol_service_dataservice_write,
        &conn->svc->loop);
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

    /* remove the connection from the event loop. */
    ipc_event_loop_remove(&svc->loop, &conn->ctx);

    /* if still associated with a dataservice child context, remove it. */
    if (conn->dataservice_child_context >= 0)
    {
        /* send a child context close request to the dataservice. */
        dataservice_api_sendreq_child_context_close(
            &svc->data, conn->dataservice_child_context);

        /* set the write callback for the dataservice socket. */
        ipc_set_writecb_noblock(
            &svc->data, &unauthorized_protocol_service_dataservice_write,
            &svc->loop);

        svc->dataservice_child_map[conn->dataservice_child_context] = NULL;
        conn->dataservice_child_context = -1;
    }

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
        + conn->svc->agent_pubkey.size + conn->server_key_nonce.size + conn->server_challenge_nonce.size + conn->svc->suite.mac_short_opts.mac_size;

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
        vccrypt_suite_mac_short_init(
            &conn->svc->suite, &mac, &conn->shared_secret);
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
    /* TODO - there should be a suite buffer init method for this. */
    vccrypt_buffer_t mac_buffer;
    retval =
        vccrypt_buffer_init(
            &mac_buffer, &conn->svc->alloc_opts,
            conn->svc->suite.mac_short_opts.mac_size);
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
        &conn->ctx, &unauthorized_protocol_service_connection_write,
        &conn->svc->loop);

    /* set the IVs. */
    conn->client_iv = 0x0000000000000001UL;
    conn->server_iv = 0x8000000000000001UL;

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
 * \param encrypted     Set to true if this packet should be encrypted.
 */
static void unauthorized_protocol_service_error_response(
    unauthorized_protocol_connection_t* conn, int request_id, int status,
    uint32_t offset, bool encrypted)
{
    int retval = AGENTD_STATUS_SUCCESS;

    uint32_t payload[3] = { htonl(request_id), htonl(status), htonl(offset) };

    /* attempt to write the response payload to the socket. */
    if (encrypted)
    {
        /* encrypted write. */
        retval =
            ipc_write_authed_data_noblock(
                &conn->ctx, conn->server_iv, payload, sizeof(payload),
                &conn->svc->suite, &conn->shared_secret);

        /* Update the server iv. */
        ++conn->server_iv;
    }
    else
    {
        /* unencrypted write. */
        retval = ipc_write_data_noblock(&conn->ctx, payload, sizeof(payload));
    }

    /* verify the status of writing this error response. */
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        unauthorized_protocol_service_close_connection(conn);
        return;
    }

    /* set state to unauthorized so we disconnect after writing the error. */
    conn->state = UPCS_UNAUTHORIZED;

    /* set the write callback for the protocol socket. */
    ipc_set_writecb_noblock(
        &conn->ctx, &unauthorized_protocol_service_connection_write,
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
static void unauthorized_protocol_service_connection_write(
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
        /* force the write callback to happen again. */
        else
        {
            ipc_set_writecb_noblock(
                &conn->ctx, &unauthorized_protocol_service_connection_write,
                &conn->svc->loop);
        }
    }
    else
    {
        /* we are done writing to this socket. */
        ipc_set_writecb_noblock(
            &conn->ctx, NULL, &conn->svc->loop);

        /* should we change state? */
        switch (conn->state)
        {
            /* after writing the handshake response to the client, read the
             * ack. */
            case UPCS_WRITE_HANDSHAKE_RESP_TO_CLIENT:
                conn->state = UPCS_READ_HANDSHAKE_ACK_FROM_CLIENT;
                ipc_set_readcb_noblock(
                    &conn->ctx, &unauthorized_protocol_service_connection_read,
                    &conn->svc->loop);
                return;

            /* after writing the handshake ack response to the client, set the
             * state to authorized. */
            case UPCS_WRITE_HANDSHAKE_ACK_TO_CLIENT:
                unauthorized_protocol_service_dataservice_request_child_context(
                    conn);
                return;

            /* after writing a command response to the client, reset. */
            case APCS_WRITE_COMMAND_RESP_TO_CLIENT:
                conn->state = APCS_READ_COMMAND_REQ_FROM_CLIENT;
                ipc_set_readcb_noblock(
                    &conn->ctx, &unauthorized_protocol_service_connection_read,
                    &conn->svc->loop);
                return;

            /* if we are in a forced unauthorized state, close the
             * connection. */
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
 * \brief Read data from the random service socket.
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
            conn, UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE, status, 0, false);
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

/**
 * \brief Write data to the dataservice socket.
 */
static void unauthorized_protocol_service_dataservice_write(
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
            /* TODO - shut down the service.  This shouldn't happen. */
            return;
        }
    }
    else
    {
        ipc_set_writecb_noblock(&svc->data, NULL, &svc->loop);
    }
}

/**
 * \brief Read data from the data service socket.
 */
static void unauthorized_protocol_service_dataservice_read(
    ipc_socket_context_t* UNUSED(ctx), int UNUSED(event_flags),
    void* user_context)
{
    uint32_t* resp = NULL;
    uint32_t resp_size = 0U;

    /* get the instance from the user context. */
    unauthorized_protocol_service_instance_t* svc =
        (unauthorized_protocol_service_instance_t*)user_context;

    /* attempt to read a response packet. */
    int retval = ipc_read_data_noblock(&svc->data, (void**)&resp, &resp_size);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK == retval)
    {
        return;
    }
    /* handle general failures from the data service socket read. */
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        /* TODO - shut down the service. */
        return;
    }

    /* verify that the size is at least large enough for a method. */
    if (resp_size < sizeof(uint32_t))
    {
        /* TODO - shut down service on corrupt data socket? */
        goto cleanup_resp;
    }

    /* decode the method. */
    uint32_t method = ntohl(resp[0]);

    /* dispatch the method. */
    switch (method)
    {
        /* child context create response. */
        case DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE:
            ups_dispatch_dataservice_response_child_context_create(
                svc, resp, resp_size);
            break;

        /* child context close response. */
        case DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE:
            ups_dispatch_dataservice_response_child_context_close(
                svc, resp, resp_size);
            break;

        /* latest block read response. */
        case DATASERVICE_API_METHOD_APP_BLOCK_ID_LATEST_READ:
            ups_dispatch_dataservice_response_block_id_latest_read(
                svc, resp, resp_size);
            break;

        /* unknown method. */
        default:
            /* TODO - if this happens after everything is decoded, log and shut
             * down service. */
            break;
    }

cleanup_resp:
    memset(resp, 0, resp_size);
    free(resp);
}

/**
 * Handle a child_context_create response.
 */
static void ups_dispatch_dataservice_response_child_context_create(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size)
{
    dataservice_response_child_context_create_t dresp;

    /* decode the response. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_response_child_context_create(
            resp, resp_size, &dresp))
    {
        /* TODO - handle failure. */
        return;
    }

    /* TODO - select based on client ID. */
    unauthorized_protocol_connection_t* conn =
        svc->dataservice_context_create_head;
    if (NULL == conn)
    {
        /* TODO - we should indicate an error here. */
        goto cleanup_dresp;
    }

    /* remove the context from the dataservice wait queue and add to connection
     * queue. */
    unauthorized_protocol_connection_remove(
        &svc->dataservice_context_create_head, conn);
    unauthorized_protocol_connection_push_front(
        &svc->used_connection_head, conn);

    /* save the context in the connection. */
    conn->dataservice_child_context = dresp.child;

    /* save the connection to the context array. */
    svc->dataservice_child_map[dresp.child] = conn;

    /* evolve the state of the connection. */
    conn->state = APCS_READ_COMMAND_REQ_FROM_CLIENT;

    /* set connection read callback to read request from client. */
    ipc_set_readcb_noblock(
        &conn->ctx, &unauthorized_protocol_service_connection_read,
        &conn->svc->loop);

cleanup_dresp:
    dispose((disposable_t*)&dresp);
}

/**
 * Handle a child_context_close response.
 */
static void ups_dispatch_dataservice_response_child_context_close(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size)
{
    (void)svc;
    (void)resp;
    (void)resp_size;
}

#include <stdio.h>

/**
 * Handle a block_id_latest_read response.
 */
static void ups_dispatch_dataservice_response_block_id_latest_read(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size)
{
    dataservice_response_latest_block_id_get_t dresp;

    /* decode the response. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_response_latest_block_id_get(
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
    uint32_t net_method = htonl(UNAUTH_PROTOCOL_REQ_ID_LATEST_BLOCK_GET);
    uint32_t net_status = dresp.hdr.status;
    uint32_t net_offset = conn->current_request_offset;
    uint8_t payload[3 * sizeof(uint32_t) + 16];
    memcpy(payload, &net_method, 4);
    memcpy(payload + 4, &net_status, 4);
    memcpy(payload + 8, &net_offset, 4);
    memcpy(payload + 12, dresp.block_id, 16);

    /* attempt to write this payload to the socket. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_write_authed_data_noblock(
            &conn->ctx, conn->server_iv, payload, sizeof(payload),
            &conn->svc->suite, &conn->shared_secret))
    {
        unauthorized_protocol_service_close_connection(conn);
        goto cleanup_dresp;
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

/**
 * \brief Request that a dataservice child context be created.
 *
 * \param conn      The connection to be assigned a child context when this
 *                  request completes.
 *
 * This connection is pushed to the dataservice context create list, where it
 * will remain until the next dataservice context create request completes.
 */
static void unauthorized_protocol_service_dataservice_request_child_context(
    unauthorized_protocol_connection_t* conn)
{
    unauthorized_protocol_service_instance_t* svc =
        (unauthorized_protocol_service_instance_t*)conn->svc;

    /* remove the connection from the connection list. */
    unauthorized_protocol_connection_remove(
        &svc->used_connection_head, conn);
    /* place the connection onto the dataservice connection wait head. */
    unauthorized_protocol_connection_push_front(
        &svc->dataservice_context_create_head, conn);

    /* set the client connection state to wait for the child context. */
    conn->state = APCS_DATASERVICE_CHILD_CONTEXT_WAIT;

    /* TODO - derive bitset from client authorization. */
    BITCAP_INIT_FALSE(conn->dataservice_caps);
    BITCAP_SET_TRUE(
        conn->dataservice_caps, DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(
        conn->dataservice_caps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /*
     * TODO - we need a way to tie a unique ID (i.e. client UUID) to the client
     * context to ensure that we can't get a race to escalate privileges.  For
     * now, clients have 100% access to the API.  THIS IS DEMO ONLY AND NOT
     * PRODUCTION HARDENED.
     */

    /* send a child context create request to the dataservice. */
    dataservice_api_sendreq_child_context_create(
        &svc->data, conn->dataservice_caps, sizeof(conn->dataservice_caps));

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        &svc->data, &unauthorized_protocol_service_dataservice_write,
        &svc->loop);
}
