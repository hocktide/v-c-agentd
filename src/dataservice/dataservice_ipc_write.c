/**
 * \file dataservice/dataservice_ipc_write.c
 *
 * \brief Write callback for the database protocol socket.
 *
 * \copyright 2018-2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Write callback for the data service protocol socket.
 *
 * This callback is registered as part of the ipc callback mechanism for the
 * data service protocol socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this random socket.
 */
void dataservice_ipc_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    dataservice_instance_t* instance = (dataservice_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_READ);
    MODEL_ASSERT(NULL != instance);

    /* write data if there is data to write. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        /* attempt to write data. */
        int bytes_written = ipc_socket_write_from_buffer(ctx);

        /* has the socket been closed? */
        if (0 == bytes_written)
        {
            goto exit_failure;
        }
        /* has there been a socket error? */
        else if (bytes_written < 0)
        {
            /* for any error except retrying, exit the loop. */
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                goto exit_failure;
            }
        }
        /* re-enable callback if there is more data to write. */
        else if (ipc_socket_writebuffer_size(ctx) > 0)
        {
            ipc_set_writecb_noblock(
                ctx, &dataservice_ipc_write, instance->loop_context);
        }
    }
    else
    {
        /* disable callback if there is no more data to write. */
        ipc_set_writecb_noblock(
            ctx, NULL, instance->loop_context);
    }

    /* success. */
    return;

exit_failure:
    dataservice_exit_event_loop(instance);
}
