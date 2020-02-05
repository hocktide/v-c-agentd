/**
 * \file ipc/ipc_timer_init.c
 *
 * \brief Initialize an IPC timer.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/* forward decls */
static void ipc_timer_context_dispose(void* disposable);

/**
 * \brief Initialize a timer.
 *
 * On success, sd is a one-shot timer of the given duration which can be added
 * to the event loop.
 *
 * \param timer         The timer to initialize.
 * \param milliseconds  The number of milliseconds before the timer expires.
 * \param cb            The callback to use when this timer expires.
 * \param user_context  The user context for this timer.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition
 *        occurred during this operation.
 */
int ipc_timer_init(
    ipc_timer_context_t* timer, uint64_t milliseconds, ipc_timer_event_cb_t cb,
    void* user_context)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != timer);
    MODEL_ASSERT(milliseconds > 0);
    MODEL_ASSERT(NULL != cb);

    /* attempt to allocate an impl structure for this timer. */
    ipc_timer_impl_t* impl =
        (ipc_timer_impl_t*)malloc(sizeof(ipc_timer_impl_t));
    if (NULL == impl)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* clear this structure. */
    memset(impl, 0, sizeof(ipc_timer_impl_t));

    /* set up the timer context. */
    memset(timer, 0, sizeof(ipc_timer_context_t));
    timer->hdr.dispose = &ipc_timer_context_dispose;
    timer->milliseconds = milliseconds;
    timer->impl = impl;
    timer->callback = cb;
    timer->user_context = user_context;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Dispose of a timer context.
 *
 * \param disposable            The timer context to dispose.
 */
static void ipc_timer_context_dispose(void* disposable)
{
    ipc_timer_context_t* timer = (ipc_timer_context_t*)disposable;
    ipc_timer_impl_t* impl = (ipc_timer_impl_t*)timer->impl;

    /* sanity checks. */
    MODEL_ASSERT(NULL != timer);
    MODEL_ASSERT(NULL != impl);

    /* if the timer event is set for this timer, free it. */
    if (NULL != impl->timer_ev)
    {
        event_free(impl->timer_ev);
        impl->timer_ev = NULL;
    }

    /* free the impl. */
    free(impl);

    /* clear the structure. */
    memset(timer, 0, sizeof(ipc_timer_context_t));
}
