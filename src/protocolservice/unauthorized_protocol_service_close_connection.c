/**
 * \file protocolservice/unauthorized_protocol_service_close_connection.c
 *
 * \brief Close a client connection.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Close a connection, returning it to the free connection pool.
 *
 * \param conn          The connection to close.
 */
void unauthorized_protocol_service_close_connection(
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
