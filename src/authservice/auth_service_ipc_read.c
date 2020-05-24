/**
 * \file authservice/auth_service_ipc_read.c
 *
 * \brief Read callback for the auth service protocol socket.
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
 * \brief Read callback for the auth service protocol socket.
 *
 * This callback is registered as part of the ipc callback mechanism for the
 * auth service protocol socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this random socket.
 */
void auth_service_ipc_read(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    ssize_t retval = 0;
    void* req;
    uint32_t size = 0;
    auth_service_instance_t* instance = (auth_service_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_READ);
    MODEL_ASSERT(NULL != instance);

    /* don't process data from this socket if we have been forced to exit. */
    if (instance->auth_service_force_exit)
        return;

    /* attempt to read a request. */
    retval = ipc_read_data_noblock(ctx, &req, &size);
    switch (retval)
    {
        /* on success, decode and dispatch. */
        case 0:
            if (0 !=
                auth_service_decode_and_dispatch(instance, ctx, req, size))
            {
                auth_service_exit_event_loop(instance);
            }

            /* clear and free the request data. */
            memset(req, 0, size);
            free(req);
            break;

        /* Wait for more data on the socket. */
        case AGENTD_ERROR_IPC_WOULD_BLOCK:
            break;

        /* any other error code indicates that we should no longer trust the
         * socket. */
        default:
            auth_service_exit_event_loop(instance);
            break;
    }

    /* fire up the write callback if there is data to write. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        ipc_set_writecb_noblock(ctx, &auth_service_ipc_write, instance->loop);
    }
}
