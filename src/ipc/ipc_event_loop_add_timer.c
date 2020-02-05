/**
 * \file ipc/ipc_event_loop_add_timer.c
 *
 * \brief Add a timer event to the event loop.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/**
 * \brief Add a timer to the event loop.
 *
 * On success, the event loop will notify the callback associated with this
 * timer when it goes off.
 *
 * \param loop          The event loop context to which this timer is added.
 * \param timer         The timer context to add to the event loop.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_INVALID_ARGUMENT if the timer context has already
 *        been added to an event loop.
 *      - AGENTD_ERROR_IPC_EVENT_ADD_FAILURE if the event cannot be added to the
 *        event loop.
 */
int ipc_event_loop_add_timer(
    ipc_event_loop_context_t* loop, ipc_timer_context_t* timer)
{
    ssize_t retval = 0;

    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != loop);
    MODEL_ASSERT(NULL != timer);

    /* get the impls. */
    ipc_event_loop_impl_t* loop_impl = (ipc_event_loop_impl_t*)loop->impl;
    ipc_timer_impl_t* timer_impl = (ipc_timer_impl_t*)timer->impl;

    /* clean up the timer event if already set. */
    if (NULL != timer_impl->timer_ev)
    {
        event_free(timer_impl->timer_ev);
        timer_impl->timer_ev = NULL;
    }

    /* create the read event. */
    timer_impl->timer_ev =
        evtimer_new(
            loop_impl->evb, &ipc_event_loop_cb, timer);
    if (NULL == timer_impl->timer_ev)
    {
        retval = AGENTD_ERROR_IPC_EVENT_NEW_FAILURE;
        goto done;
    }

    /* calculate the event timer time. */
    struct timeval evtime;
    evtime.tv_sec = timer->milliseconds / 1000;
    evtime.tv_usec = (timer->milliseconds % 1000) * 1000;

    /* add the event to the event base. */
    if (0 != evtimer_add(timer_impl->timer_ev, &evtime))
    {
        retval = AGENTD_ERROR_IPC_EVENT_ADD_FAILURE;
        goto cleanup_timer_ev;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

cleanup_timer_ev:
    if (NULL != timer_impl->timer_ev)
    {
        event_free(timer_impl->timer_ev);
        timer_impl->timer_ev = NULL;
    }

done:
    return retval;
}
