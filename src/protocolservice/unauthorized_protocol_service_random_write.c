/**
 * \file protocolservice/unauthorized_protocol_service_random_write.c
 *
 * \brief Write to the random service socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <errno.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Write data to the random service socket.
 *
 * \param ctx           The socket context triggering this event.
 * \param event_flags   The flags for this event.
 * \param user_context  The user context for this event.
 */
void unauthorized_protocol_service_random_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    /* get the instance from the user context. */
    unauthorized_protocol_service_instance_t* svc =
        (unauthorized_protocol_service_instance_t*)user_context;

    /* first, see if we still need to write data. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        /* attempt to write data. */
        int bytes_written = ipc_socket_write_from_buffer(ctx);

        /* was the socket closed, or was there an error? */
        if (bytes_written == 0 || (bytes_written < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)))
        {
            unauthorized_protocol_service_exit_event_loop(svc);
            return;
        }
        /* if there is more data to write, keep the write callback active. */
        else if (ipc_socket_writebuffer_size(ctx) > 0)
        {
            ipc_set_writecb_noblock(
                &svc->random, &unauthorized_protocol_service_random_write,
                &svc->loop);
        }
    }
    else
    {
        ipc_set_writecb_noblock(&svc->random, NULL, &svc->loop);
    }
}
