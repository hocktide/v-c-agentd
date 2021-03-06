/**
 * \file listenservice/listenservice_event_loop.c
 *
 * \brief The event loop for the listen service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <signal.h>
#include <vpr/parameters.h>

#include "listenservice_internal.h"

/**
 * \brief Event loop for the unauthorized listen service.  This is the entry
 * point for the listen service.  It handles the details of reacting to events
 * sent over the listen service socket.
 *
 * \param logsock       The logging service socket.  The listen service logs
 *                      on this socket.
 * \param acceptsock    The socket to which newly accepted sockets are sent.
 * \param listenstart   The first socket to which this service will listen.  The
 *                      listen service will iterate from this socket until it
 *                      encounters a closed descriptor and use each as a listen
 *                      socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_LISTENSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *            attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_LISTENSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_LISTENSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if adding
 *            the listen service socket to the event loop failed.
 *          - AGENTD_ERROR_LISTENSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if running
 *            the listen service event loop failed.
 */
int listenservice_event_loop(
    int UNUSED(logsock), int acceptsock, int listenstart)
{
    int retval = 0;
    ipc_socket_context_t* listensockets;
    ipc_event_loop_context_t loop;
    listenservice_instance_t instance;

    /* parameter sanity checking. */
    MODEL_ASSERT(logsock >= 0);
    MODEL_ASSERT(listenstart >= 0);

    /* count the number of listen sockets. */
    int listensocket_count = listenservice_count_sockets(listenstart);

    /* allocate memory for the listen sockets. */
    listensockets = (ipc_socket_context_t*)
        malloc(listensocket_count * sizeof(ipc_socket_context_t));
    if (NULL == listensockets)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* initialize an IPC event loop instance. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_init(&loop))
    {
        retval = AGENTD_ERROR_LISTENSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto free_listensockets;
    }

    /* set up instance. */
    memset(&instance, 0, sizeof(instance));
    /* set a reference to the event loop in the instance. */
    instance.loop_context = &loop;
    instance.acceptsock = acceptsock;

    /* on these signals, leave the event loop and shut down gracefully. */
    ipc_exit_loop_on_signal(&loop, SIGHUP);
    ipc_exit_loop_on_signal(&loop, SIGTERM);
    ipc_exit_loop_on_signal(&loop, SIGQUIT);

    /* iterate through all of the listen sockets. */
    for (int i = 0; i < listensocket_count; ++i)
    {
        /* set the listen socket to non-blocking. */
        if (AGENTD_STATUS_SUCCESS !=
            ipc_make_noblock(listenstart + i, listensockets + i, &instance))
        {
            retval = AGENTD_ERROR_DATASERVICE_IPC_MAKE_NOBLOCK_FAILURE;
            goto cleanup_loop;
        }

        /* set the read, write, and error callbacks for the data socket. */
        ipc_set_readcb_noblock(
            listensockets + i, &listenservice_ipc_accept, NULL);

        /* add the listen socket to the event loop. */
        if (AGENTD_STATUS_SUCCESS !=
            ipc_event_loop_add(&loop, listensockets + i))
        {
            retval = AGENTD_ERROR_LISTENSERVICE_IPC_EVENT_LOOP_ADD_FAILURE;
            goto cleanup_loop;
        }
    }

    /* run the ipc event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_run(&loop))
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_EVENT_LOOP_RUN_FAILURE;
        goto cleanup_listensockets;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_listensockets:
    for (int i = 0; i < listensocket_count; ++i)
    {
        ipc_event_loop_remove(&loop, listensockets + i);
        dispose((disposable_t*)listensockets + i);
    }

cleanup_loop:
    dispose((disposable_t*)&loop);

free_listensockets:
    free(listensockets);

done:
    return retval;
}
