/**
 * \file ipc/ipc_exit_loop_on_signal.c
 *
 * \brief Set the event loop to exit when a given signal is caught.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/* forward decls */
static void ipc_signal_cb(evutil_socket_t, short, void*);

/**
 * \brief Exit the event loop when the given signal is caught.
 *
 * On success, the event loop will exit when this signal is caught by the signal
 * handler.
 *
 * \param loop          The event loop context to exit when the signal is
 *                      caught.
 * \param sig           The signal that triggers this exit.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered.
 *      - AGENTD_ERROR_IPC_EVSIGNAL_NEW_FAILURE if a new signal event could not
 *        be created.
 *      - AGENTD_ERROR_IPC_EVENT_ADD_FAILURE if the signal event could not be
 *        added to the event base.
 */
int ipc_exit_loop_on_signal(
    ipc_event_loop_context_t* loop, int sig)
{
    ssize_t retval = 0;

    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != loop);

    /* get the impls. */
    ipc_event_loop_impl_t* loop_impl = (ipc_event_loop_impl_t*)loop->impl;

    /* create an event structure. */
    ipc_signal_event_impl_t* sigev =
        (ipc_signal_event_impl_t*)malloc(sizeof(ipc_signal_event_impl_t));
    if (NULL == sigev)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* create the event for this signal. */
    sigev->ev = evsignal_new(loop_impl->evb, sig, &ipc_signal_cb, loop);
    if (NULL == sigev->ev)
    {
        retval = AGENTD_ERROR_IPC_EVSIGNAL_NEW_FAILURE;
        goto cleanup_sigev;
    }

    /* add the event to the event base. */
    if (0 != event_add(sigev->ev, NULL))
    {
        retval = AGENTD_ERROR_IPC_EVENT_ADD_FAILURE;
        goto cleanup_event;
    }

    /* add this event structure to our loop. */
    sigev->next = loop_impl->sig_head;
    loop_impl->sig_head = sigev;

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

cleanup_event:
    event_free(sigev->ev);
    sigev->ev = NULL;

cleanup_sigev:
    free(sigev);

done:
    return retval;
}

/**
 * \brief Event loop callback.  Decode an event and send it to the ipc callback.
 *
 * \param fd        The socket file descriptor for this callback.
 * \param what      The flags for this event.
 * \param ctx       The user context for this event.
 */
static void ipc_signal_cb(
    evutil_socket_t UNUSED(fd), short UNUSED(what), void* ctx)
{
    /* get the socket context. */
    ipc_event_loop_context_t* loop = (ipc_event_loop_context_t*)ctx;
    ipc_event_loop_impl_t* loopimpl = (ipc_event_loop_impl_t*)loop->impl;

    /* trigger the event loop to exit. */
    event_base_loopexit(loopimpl->evb, NULL);
}
