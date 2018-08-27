/**
 * \file ipc/ipc_socket_read_to_buffer.c
 *
 * \brief Read data to the read buffer from the socket.
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
 * \brief Read data from the socket and place this into the readbuffer.
 *
 * \note This method can only be called after a socket has been added to the
 * event loop.  Otherwise, the result is undefined.
 *
 * \param sock          The non-blocking socket for the read.
 *
 * \returns the number of bytes read, -1 on error, or 0.  If 0 is returned
 * AND the socket is available for reading via a read callback, then this
 * indicates that the socket has been closed by the peer.  If -1 is returned,
 * then errno should be checked to see if this is a real error or if the read
 * failed because it would block (EAGAIN / EWOULDBLOCK).
 */
ssize_t ipc_socket_read_to_buffer(ipc_socket_context_t* sock)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != sock->impl);
    MODEL_ASSERT(NULL != ((ipc_socket_impl_t*)sock->impl)->readbuf);

    /* get the socket impl. */
    ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

    /* we can't perform a write using an invalid buffer. */
    if (NULL == sock_impl->readbuf)
    {
        errno = EFAULT;
        return -1;
    }

    /* use libevent's read method. */
    return evbuffer_read(sock_impl->readbuf, sock->fd, -1);
}
