/**
 * \file canonization/canonizationservice_event_loop.c
 *
 * \brief The event loop for the canonization service.
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <agentd/canonizationservice.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <errno.h>
#include <signal.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Event loop for the canonization service.  This is the entry point for
 * the canonization service.
 *
 * \param datasock      The data service socket.  The canonization service
 *                      communicates with the dataservice using this socket.
 * \param randomsock    The random service socket.  The canonization service
 *                      communicates with the random service using this socket.
 * \param logsock       The logging service socket.  The canonization service
 *                      logs on this socket.
 * \param controlsock   The socket used to control the canonization service.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *            attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if
 *            adding the protocol service socket to the event loop failed.
 *          - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if
 *            running the protocol service event loop failed.
 */
int canonizationservice_event_loop(
    int datasock, int randomsock, int UNUSED(logsock), int controlsock)
{
    int retval = AGENTD_STATUS_SUCCESS;
    canonizationservice_instance_t* instance = NULL;
    ipc_socket_context_t data;
    ipc_socket_context_t random;
    ipc_socket_context_t control;
    ipc_event_loop_context_t loop;

    /* parameter sanity checking. */
    MODEL_ASSERT(datasock >= 0);
    MODEL_ASSERT(randomsock >= 0);
    MODEL_ASSERT(logsock >= 0);
    MODEL_ASSERT(controlsock >= 0);

    /* Create the canonization service instance. */
    instance = canonizationservice_instance_create();
    if (NULL == instance)
    {
        retval = AGENTD_ERROR_CANONIZATIONSERVICE_INSTANCE_CREATE_FAILURE;
        goto done;
    }

    /* set the control socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_make_noblock(controlsock, &control, instance))
    {
        retval = AGENTD_ERROR_CANONIZATIONSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_instance;
    }

    /* set the data socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_make_noblock(datasock, &data, instance))
    {
        retval = AGENTD_ERROR_CANONIZATIONSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_control_socket;
    }

    /* save the data socket context for use by instance methods. */
    instance->data = &data;

    /* set the random socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_make_noblock(randomsock, &random, instance))
    {
        retval = AGENTD_ERROR_CANONIZATIONSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_data_socket;
    }

    /* save the random socket context for use by instance methods. */
    instance->random = &random;

    /* initialize the IPC event loop instance. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_init(&loop))
    {
        retval = AGENTD_ERROR_CANONIZATIONSERVICE_IPC_EVENT_LOOP_INIT;
        goto cleanup_random_socket;
    }

    /* set a reference to the event loop in the instance. */
    instance->loop_context = &loop;

    /* set the read callback on the sockets. */
    ipc_set_readcb_noblock(&control, &canonizationservice_control_read, NULL);
    ipc_set_readcb_noblock(&data, &canonizationservice_data_read, NULL);
    ipc_set_readcb_noblock(&random, &canonizationservice_random_read, NULL);

    /* on these signals, leave the event loop and shut down gracefully. */
    ipc_exit_loop_on_signal(&loop, SIGHUP);
    ipc_exit_loop_on_signal(&loop, SIGTERM);
    ipc_exit_loop_on_signal(&loop, SIGQUIT);

    /* add the control socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(&loop, &control))
    {
        retval = AGENTD_ERROR_CANONIZATIONSERVICE_IPC_EVENT_LOOP_ADD_FAILURE;
        goto cleanup_loop;
    }

    /* add the data socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(&loop, &data))
    {
        retval = AGENTD_ERROR_CANONIZATIONSERVICE_IPC_EVENT_LOOP_ADD_FAILURE;
        goto cleanup_loop;
    }

    /* add the random socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(&loop, &random))
    {
        retval = AGENTD_ERROR_CANONIZATIONSERVICE_IPC_EVENT_LOOP_ADD_FAILURE;
        goto cleanup_loop;
    }

    /* set the initial state for the canonization service. */
    instance->state = CANONIZATIONSERVICE_STATE_IDLE;

    /* run the ipc event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_run(&loop))
    {
        retval = AGENTD_ERROR_CANONIZATIONSERVICE_IPC_EVENT_LOOP_RUN_FAILURE;
        goto cleanup_loop;
    }

cleanup_loop:
    dispose((disposable_t*)&loop);

cleanup_random_socket:
    dispose((disposable_t*)&random);

cleanup_data_socket:
    dispose((disposable_t*)&data);

cleanup_control_socket:
    dispose((disposable_t*)&control);

cleanup_instance:
    if (NULL != instance)
    {
        dispose((disposable_t*)instance);
        free(instance);
    }

done:
    return retval;
}
