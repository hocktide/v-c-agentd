/**
 * \file authservice/auth_service_ipc_write.c
 *
 * \brief Write callback for the auth service protocol socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/authservice/private/authservice.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <errno.h>
#include <string.h>
#include <vpr/parameters.h>

#include "auth_service_private.h"

/**
 * \brief Write callback for the auth service protocol socket.
 *
 * This callback is registered as part of the ipc callback mechanism for the
 * auth service protocol socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this random socket.
 */
void auth_service_ipc_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    auth_service_instance_t* instance = (auth_service_instance_t*)user_context;

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
        /* re-enable callback if there is still data to write. */
        else if (ipc_socket_writebuffer_size(ctx) > 0)
        {
            ipc_set_writecb_noblock(
                ctx, &auth_service_ipc_write, instance->loop);
        }
    }
    else
    {
        /* disable callback if there is no more data to write. */
        ipc_set_writecb_noblock(
            ctx, NULL, instance->loop);
    }

    /* success. */
    return;

exit_failure:
    auth_service_exit_event_loop(instance);
}
