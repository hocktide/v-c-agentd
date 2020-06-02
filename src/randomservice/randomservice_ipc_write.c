/**
 * \file dataservice/randomservice_ipc_write.c
 *
 * \brief Write callback for the random service protocol socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/private/randomservice.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "randomservice_internal.h"

/**
 * \brief Write callback for the random service protocol socket.
 *
 * This callback is registered as part of the ipc callback mechanism for the
 * random service protocol socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this random socket.
 */
void randomservice_ipc_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    randomservice_root_context_t* instance =
        (randomservice_root_context_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_WRITE);
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
        /* has there been an unrecoverable socket error? */
        else if (bytes_written < 0
                 && (errno != EAGAIN && errno != EWOULDBLOCK))
        {
            goto exit_failure;
        }
        /* if there is more data to write, re-activate the write callback. */
        else if (ipc_socket_writebuffer_size(ctx) > 0)
        {
            ipc_set_writecb_noblock(
                ctx, &randomservice_ipc_write, instance->loop_context);
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
    randomservice_exit_event_loop(instance);
}
