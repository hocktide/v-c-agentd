/**
 * \file ipc/ipc_event_loop_run.c
 *
 * \brief Run the event loop.
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

/**
 * \brief Run the event loop for IPC non-blocking I/O.
 *
 * \param loop          The event loop context to run.
 *
 * \returns 0 on success and non-zero on failure.
 */
int ipc_event_loop_run(ipc_event_loop_context_t* loop)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != loop);

    /* get the loop impl. */
    ipc_event_loop_impl_t* loop_impl = (ipc_event_loop_impl_t*)loop->impl;

    /* return the status code from running the event loop. */
    return event_base_loop(loop_impl->evb, 0);
}
