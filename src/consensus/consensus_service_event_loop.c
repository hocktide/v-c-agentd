/**
 * \file protocolservice/consensus_service_event_loop.c
 *
 * \brief The event loop for the consensus service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <agentd/consensusservice.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <signal.h>
#include <vpr/parameters.h>

/**
 * \brief Event loop for the consensus service.  This is the entry point for the
 * consensus service.
 *
 * \param datasock      The data service socket.  The protocol service
 *                      communicates with the dataservice using this socket.
 * \param logsock       The logging service socket.  The protocol service logs
 *                      on this socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_CONSENSUSSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *            attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if adding
 *            the protocol service socket to the event loop failed.
 *          - AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if
 *            running the protocol service event loop failed.
 */
int consensus_service_event_loop(
    int UNUSED(datasock), int UNUSED(logsock))
{
    int retval = AGENTD_STATUS_SUCCESS;
    ipc_event_loop_context_t loop;

    /* initialize the IPC event loop instance. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_init(&loop))
    {
        retval = AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_INIT;
        goto done;
    }

    /* on these signals, leave the event loop and shut down gracefully. */
    ipc_exit_loop_on_signal(&loop, SIGHUP);
    ipc_exit_loop_on_signal(&loop, SIGTERM);
    ipc_exit_loop_on_signal(&loop, SIGQUIT);

    /* run the ipc event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_run(&loop))
    {
        retval = AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_RUN_FAILURE;
        goto cleanup_loop;
    }

cleanup_loop:
    dispose((disposable_t*)&loop);

done:
    return retval;
}
