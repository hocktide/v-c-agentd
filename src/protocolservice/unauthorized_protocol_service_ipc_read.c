/**
 * \file protocolservice/unauthorized_protocol_service_ipc_read.c
 *
 * \brief Read socket connections from the listener service.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Handle read events on the protocol socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this proto socket.
 */
void unauthorized_protocol_service_ipc_read(
    ipc_socket_context_t* ctx, int UNUSED(event_flags),
    void* user_context)
{
    int recvsock;

    /* TODO - This should be non-blocking on the listener and protocol side. */

    /* get the instance from the user context. */
    unauthorized_protocol_service_instance_t* inst =
        (unauthorized_protocol_service_instance_t*)user_context;

    /* don't accept any more sockets if we're shutting down. */
    if (inst->force_exit)
        return;

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
