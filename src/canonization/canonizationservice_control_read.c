/**
 * \file canonization/canonizationservice_control_read.c
 *
 * \brief Read data from the control socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Handle read events on the control socket.
 *
 * \param ctx               The non-blocking socket context.
 * \param event_flags       The event that triggered this callback.
 * \param user_context      The user context for this control socket.
 */
void canonizationservice_control_read(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    int retval = 0;
    void* req;
    uint32_t size = 0;
    canonizationservice_instance_t* instance =
        (canonizationservice_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_READ);
    MODEL_ASSERT(NULL != instance);

    /* don't process data from this socket if we have been forced to exit. */
    if (instance->force_exit)
        return;

    /* attempt to read a request. */
    retval = ipc_read_data_noblock(ctx, &req, &size);
    switch (retval)
    {
        /* on success, decode and dispatch. */
        case AGENTD_STATUS_SUCCESS:
            if (AGENTD_STATUS_SUCCESS !=
                canonizationservice_decode_and_dispatch_control_command(
                    instance, ctx, req, size))
            {
                /* a bad control message means we should shut down. */
                instance->force_exit = true;
                ipc_exit_loop(instance->loop_context);
            }

            /* clear and free the request data. */
            memset(req, 0, size);
            free(req);
            break;

        /* wait for more data on the socket. */
        case AGENTD_ERROR_IPC_WOULD_BLOCK:
            break;

        /* any other error code indicates that we should no longer trust the
         * control socket. */
        default:
            instance->force_exit = true;
            ipc_exit_loop(instance->loop_context);
            break;
    }

    /* fire up the write callback if there is data to write. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        ipc_set_writecb_noblock(
            ctx, &canonizationservice_control_write, instance->loop_context);
    }
}
