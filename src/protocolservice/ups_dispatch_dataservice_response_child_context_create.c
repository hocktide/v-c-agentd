/**
 * \file
 * protocolservice/ups_dispatch_dataservice_response_child_context_create.c
 *
 * \brief Handle the response from the dataservice child context create request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * Handle a child_context_create response.
 *
 * \param svc               The protocol service instance.
 * \param resp              The response from the child context create call.
 * \param resp_size         The size of the response.
 */
void ups_dispatch_dataservice_response_child_context_create(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size)
{
    dataservice_response_child_context_create_t dresp;

    /* decode the response. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_response_child_context_create(
            resp, resp_size, &dresp))
    {
        /* TODO - log decode error. */
        unauthorized_protocol_service_exit_event_loop(svc);
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
