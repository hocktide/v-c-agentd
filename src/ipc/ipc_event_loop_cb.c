/**
 * \file ipc/ipc_event_loop_cb.c
 *
 * \brief The event loop callback for IPC methods.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/**
 * \brief Event loop callback.  Decode an event and send it to the ipc callback.
 *
 * \param fd        The socket file descriptor for this callback.
 * \param what      The flags for this event.
 * \param ctx       The user context for this event.
 */
void ipc_event_loop_cb(
    evutil_socket_t UNUSED(fd), short what, void* ctx)
{
    /* get the socket context. */
    ipc_socket_context_t* sock = (ipc_socket_context_t*)ctx;

    /* dispatch read event. */
    if ((what & EV_READ) && sock->read)
    {
        sock->read(sock, IPC_SOCKET_EVENT_READ, sock->user_context);
    }

    /* dispatch write event. */
    if ((what & EV_WRITE) && sock->write)
    {
        sock->write(sock, IPC_SOCKET_EVENT_WRITE, sock->user_context);
    }
}
