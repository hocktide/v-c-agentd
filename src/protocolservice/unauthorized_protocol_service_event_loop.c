/**
 * \file protocolservice/unauthorized_protocol_service_event_loop.c
 *
 * \brief The event loop for the unauthorized protocol service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/* forward decls */
static void unauthorized_protocol_service_ipc_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void unauthorized_protocol_service_dummy_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Event loop for the unauthorized protocol service.  This is the entry
 * point for the protocol service.  It handles the details of reacting to events
 * sent over the protocol service socket.
 *
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
    int protosock, int UNUSED(logsock))
{
    int retval = 0;
    unauthorized_protocol_service_instance_t inst;

    /* parameter sanity checking. */
    MODEL_ASSERT(protosock >= 0);
    MODEL_ASSERT(logsock >= 0);

    /* initialize this instance. */
    /* TODO - get the number of connections from config. */
    retval =
        unauthorized_protocol_service_instance_init(&inst, protosock, 50);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* set the read callback for the protocol socket. */
    ipc_set_readcb_noblock(
        &inst.proto, &unauthorized_protocol_service_ipc_read);

    /* add the protocol socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(&inst.loop, &inst.proto))
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

    /* set the write callback for this connection. */
    ipc_set_writecb_noblock(
        &conn->ctx, &unauthorized_protocol_service_dummy_write);

    /* add the socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(&inst->loop, &conn->ctx))
    {
        dispose((disposable_t*)conn);
        unauthorized_protocol_connection_push_front(
            &inst->free_connection_head, conn);
        close(recvsock);
        return;
    }

    /* this is now a used connection. */
    unauthorized_protocol_connection_push_front(
        &inst->used_connection_head, conn);

    /* write something to this socket, asynchronously. */
    ipc_write_string_noblock(&conn->ctx, "Hello, World!\n");
}

/**
 * \brief Handle write events on an unauthorized connection socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this proto socket.
 */
static void unauthorized_protocol_service_dummy_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags),
    void* user_context)
{
    /* get the instance from the user context. */
    unauthorized_protocol_connection_t* conn =
        (unauthorized_protocol_connection_t*)user_context;
    unauthorized_protocol_service_instance_t* svc =
        (unauthorized_protocol_service_instance_t*)conn->svc;

    /* write data if there is data to write. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        /* attempt to write data. */
        int bytes_written = ipc_socket_write_from_buffer(ctx);

        /* was the socket closed, or was there an error? */
        if (bytes_written == 0 || (bytes_written < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)))
        {
            goto cleanup_socket;
        }
        else
        {
            /* if we are done writing to the socket, close it out. */
            if (ipc_socket_writebuffer_size(ctx) == 0)
            {
                goto cleanup_socket;
            }
            else
            {
                return;
            }
        }
    }
    /* the socket is either closed, or an error occurred. */
    else
    {
        goto cleanup_socket;
    }

cleanup_socket:
    ipc_event_loop_remove(&svc->loop, &conn->ctx);
    unauthorized_protocol_connection_remove(
        &svc->used_connection_head, conn);
    dispose((disposable_t*)conn);
    unauthorized_protocol_connection_push_front(
        &svc->free_connection_head, conn);
}
