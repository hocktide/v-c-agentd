/**
 * \file ipc/ipc_event_loop_init.c
 *
 * \brief Initialize an event loop for non-blocking ipc.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/* forward decls */
static void ipc_event_loop_dispose(void* disposable);

/**
 * \brief Initialize the event loop for handling IPC non-blocking I/O.
 *
 * On success, this event loop is owned by the caller and must be disposed when
 * no longer needed by calling the dispose() method.
 *
 * \param loop          The event loop context to initialize.
 *
 * \returns 0 on success and non-zero on failure.
 */
int ipc_event_loop_init(ipc_event_loop_context_t* loop)
{
    int retval = 0;
    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != loop);

    /* clear the loop structure. */
    memset(loop, 0, sizeof(ipc_event_loop_context_t));

    /* allocate memory for the internal structure. */
    ipc_event_loop_impl_t* internal =
        (ipc_event_loop_impl_t*)
            malloc(sizeof(ipc_event_loop_impl_t));
    loop->impl = internal;
    if (NULL == internal)
    {
        retval = 1;
        goto done;
    }

    /* create the event base. */
    internal->evb = event_base_new();
    if (NULL == internal->evb)
    {
        retval = 2;
        goto cleanup_internal;
    }

    /* set the dispose method. */
    loop->hdr.dispose = &ipc_event_loop_dispose;

    /* success. */
    retval = 0;
    goto done;

cleanup_internal:
    free(internal);

done:
    return retval;
}

/**
 * \brief Dispose of an ipc loop context.
 *
 * \param disposable        The ipc loop context to dispose.
 */
static void ipc_event_loop_dispose(void* disposable)
{
    ipc_event_loop_context_t* ctx = (ipc_event_loop_context_t*)disposable;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(NULL != ctx->impl);

    /* get the internal context. */
    ipc_event_loop_impl_t* internal =
        (ipc_event_loop_impl_t*)ctx->impl;

    /* clean up the event. */
    event_base_free(internal->evb);

    /* free the internal structure. */
    memset(internal, 0, sizeof(ipc_event_loop_impl_t));
    free(internal);

    /* clean up the context. */
    memset(ctx, 0, sizeof(ipc_event_loop_context_t));
}
