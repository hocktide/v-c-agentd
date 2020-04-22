/**
 * \file protocolservice/unauthorized_protocol_service_event_loop.c
 *
 * \brief The event loop for the unauthorized protocol service.
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

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
