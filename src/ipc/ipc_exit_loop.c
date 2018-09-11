/**
 * \file ipc/ipc_exit_loop.c
 *
 * \brief Instruct the event loop to exit.
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
 * \brief Instruct the loop to exit as soon as all events are processed.
 *
 * \param loop          The event loop context to exit.
 */
void ipc_exit_loop(ipc_event_loop_context_t* loop)
{
    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != loop);

    /* get the impl. */
    ipc_event_loop_impl_t* loop_impl = (ipc_event_loop_impl_t*)loop->impl;

    /* trigger the event loop to exit. */
    event_base_loopexit(loop_impl->evb, NULL);
}
