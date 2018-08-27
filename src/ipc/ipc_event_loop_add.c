/**
 * \file ipc/ipc_event_loop_add.c
 *
 * \brief Add a non-blocking socket descriptor to an event loop.
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

/* forward decls */
static void ipc_event_loop_cb(evutil_socket_t, short, void*);

/**
 * \brief Add a non-blocking socket to the event loop.
 *
 * On success, the event loop will manage events on this non-blocking socket.
 * Note that the ownership for this socket context remains with the caller.  It
 * is the caller's responsibility to remove this socket from the event loop and
 * to dispose the socket.
 *
 * \param loop          The event loop context to which this socket is added.
 * \param sock          The socket context to add to the event loop.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_event_loop_add(
    ipc_event_loop_context_t* loop, ipc_socket_context_t* sock)
{
    ssize_t retval = 0;

    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != loop);
    MODEL_ASSERT(NULL != sock);

    /* get the impls. */
    ipc_event_loop_impl_t* loop_impl = (ipc_event_loop_impl_t*)loop->impl;
    ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

    /* if ev is already defined, then this socket has already been assigned. */
    if (NULL != sock_impl->ev)
    {
        retval = 1;
        goto done;
    }

    /* determine which callbacks are registered for this event. */
    short callbacks = 0;
    if (sock->read)
    {
        callbacks |= EV_READ;
    }
    if (sock->write)
    {
        callbacks |= EV_WRITE;
    }

    /* at least a read or write callback needs to be set. */
    if (0 == callbacks)
    {
        retval = 2;
        goto done;
    }

    /* create the read buffer. */
    sock_impl->readbuf = evbuffer_new();
    if (NULL == sock_impl->readbuf)
    {
        retval = 4;
        goto done;
    }

    /* create the write buffer. */
    sock_impl->writebuf = evbuffer_new();
    if (NULL == sock_impl->writebuf)
    {
        retval = 5;
        goto cleanup_readbuf;
    }

    /* create an event. */
    sock_impl->ev =
        event_new(
            loop_impl->evb, sock->fd, callbacks | EV_PERSIST, ipc_event_loop_cb,
            sock);
    if (NULL == sock_impl->ev)
    {
        retval = 6;
        goto cleanup_writebuf;
    }

    /* add the event to the event base. */
    if (0 != event_add(sock_impl->ev, NULL))
    {
        retval = 7;
        goto cleanup_event;
    }

    /* success. */
    retval = 0;
    goto done;

cleanup_event:
    event_free(sock_impl->ev);
    sock_impl->ev = NULL;

cleanup_writebuf:
    evbuffer_free(sock_impl->writebuf);
    sock_impl->writebuf = NULL;

cleanup_readbuf:
    evbuffer_free(sock_impl->readbuf);
    sock_impl->readbuf = NULL;

done:
    return retval;
}

/**
 * \brief Event loop callback.  Decode an event and send it to the ipc callback.
 *
 * \param fd        The socket file descriptor for this callback.
 * \param what      The flags for this event.
 * \param ctx       The user context for this event.
 */
static void ipc_event_loop_cb(
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
