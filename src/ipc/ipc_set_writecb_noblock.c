/**
 * \file ipc/ipc_set_writecb_noblock.c
 *
 * \brief Set the write callback for this non-blocking socket.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/**
 * \brief Set the write event callback for a non-blocking socket.
 *
 * \note If this method is called BEFORE the socket is added to the event loop,
 * it will be added as a persistent callback.  Otherwise, it is a one-shot
 * callback.
 *
 * \param sock          The socket to set.
 * \param cb            The callback to set.
 * \param loop          Optional loop context.  If set, this callback will be
 *                      added to the loop context.
 */
void ipc_set_writecb_noblock(
    ipc_socket_context_t* sock, ipc_socket_event_cb_t cb,
    ipc_event_loop_context_t* loop)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != cb);

    /* assign the write callback. */
    sock->write = cb;

    /* if the loop context is set, add the read callback. */
    if (NULL != loop)
    {
        /* get the loop implementation. */
        ipc_event_loop_impl_t* loop_impl = (ipc_event_loop_impl_t*)loop->impl;

        /* get the socket implementation. */
        ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

        /* remove the write event if set. */
        if (NULL != sock_impl->write_ev)
        {
            event_free(sock_impl->write_ev);
            sock_impl->write_ev = NULL;
        }

        /* if the callback was cleared, then we are done. */
        if (NULL == sock->write)
        {
            return;
        }

        /* otherwise, create the new write event. */
        sock_impl->write_ev =
            event_new(
                loop_impl->evb, sock->fd, EV_WRITE, &ipc_event_loop_cb, sock);
        if (NULL == sock_impl->write_ev)
        {
            /* TODO - bubble this error to the caller. */
            return;
        }

        /* add the event to the event base. */
        if (0 != event_add(sock_impl->write_ev, NULL))
        {
            /* TODO - bubble this error to the caller. */
            return;
        }
    }
}
