/**
 * \file protocolservice/unauthorized_protocol_service_dataservice_request_child_context.c
 *
 * \brief Send a child context create request to the data service.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Request that a dataservice child context be created.
 *
 * \param conn      The connection to be assigned a child context when this
 *                  request completes.
 *
 * This connection is pushed to the dataservice context create list, where it
 * will remain until the next dataservice context create request completes.
 */
void unauthorized_protocol_service_dataservice_request_child_context(
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
        conn->dataservice_caps, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
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
