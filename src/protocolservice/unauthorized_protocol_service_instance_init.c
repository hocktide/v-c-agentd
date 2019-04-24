/**
 * \file protocolservice/unauthorized_protocol_service_instance_init.c
 *
 * \brief Initialize the unauthorized protocol service instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <signal.h>
#include <string.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/* forward decls */
static void unauthorized_protocol_service_instance_dispose(void* disposable);

/**
 * \brief Create the unauthorized protocol service instance.
 *
 * \param inst          The service instance to initialize.
 * \param proto         The protocol socket to use for this instance.
 * \param max_socks     The maximum number of socket connections to accept.
 *
 * \returns a status code indicating success or failure.
 */
int unauthorized_protocol_service_instance_init(
    unauthorized_protocol_service_instance_t* inst, int proto,
    size_t max_socks)
{
    int retval = AGENTD_STATUS_SUCCESS;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(proto >= 0);
    MODEL_ASSERT(max_socks > 0);

    /* Set up the instance basics. */
    memset(inst, 0, sizeof(unauthorized_protocol_service_instance_t));
    inst->hdr.dispose = &unauthorized_protocol_service_instance_dispose;

    /* set the protocol socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS != ipc_make_noblock(proto, &inst->proto, inst))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto done;
    }

    /* initialize the IPC event loop instance. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_init(&inst->loop))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_proto;
    }

    /* on these signals, leave the event loop and shut down gracefully. */
    ipc_exit_loop_on_signal(&inst->loop, SIGHUP);
    ipc_exit_loop_on_signal(&inst->loop, SIGTERM);
    ipc_exit_loop_on_signal(&inst->loop, SIGQUIT);

    /* create connections. */
    for (size_t i = 0; i < max_socks; ++i)
    {
        /* attempt to create a connection. */
        unauthorized_protocol_connection_t* conn =
            (unauthorized_protocol_connection_t*)
                malloc(sizeof(unauthorized_protocol_connection_t));
        if (NULL == conn)
        {
            retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
            goto cleanup_connections;
        }

        /* clear this connection. */
        memset(conn, 0, sizeof(unauthorized_protocol_connection_t));

        /* add this connection to the free list. */
        unauthorized_protocol_connection_push_front(
            &inst->free_connection_head, conn);
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

cleanup_connections:
    for (unauthorized_protocol_connection_t* i = inst->free_connection_head;
         i != NULL;)
    {
        unauthorized_protocol_connection_t* next = i->next;
        free(i);
        i = next;
    }

    /*cleanup_loop:*/
    dispose((disposable_t*)&inst->loop);

cleanup_proto:
    dispose((disposable_t*)&inst->proto);

done:
    return retval;
}

/**
 * \brief Dispose of an unauthorized protocol service instance.
 */
static void unauthorized_protocol_service_instance_dispose(void* disposable)
{
    unauthorized_protocol_service_instance_t* inst =
        (unauthorized_protocol_service_instance_t*)disposable;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != inst);

    /* dispose of used conections. */
    for (unauthorized_protocol_connection_t* i = inst->used_connection_head;
         i != NULL;)
    {
        unauthorized_protocol_connection_t* next = i->next;
        dispose((disposable_t*)i);
        free(i);
        i = next;
    }

    /* dispose of free connections. */
    for (unauthorized_protocol_connection_t* i = inst->free_connection_head;
         i != NULL;)
    {
        unauthorized_protocol_connection_t* next = i->next;
        free(i);
        i = next;
    }

    /* dispose of the proto socket. */
    int protosock = inst->proto.fd;
    dispose((disposable_t*)&inst->proto);
    close(protosock);

    /* dispose of the loop. */
    dispose((disposable_t*)&inst->loop);

    /* clear this instance. */
    memset(inst, 0, sizeof(unauthorized_protocol_service_instance_t));
}
