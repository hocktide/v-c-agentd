/**
 * \file ipc/ipc_socket_readbuffer_size.c
 *
 * \brief Return the number of bytes in the read buffer.
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
 * \brief Get the number of bytes available in the read buffer.
 *
 * \note This method can only be called after a socket has been added to the
 * event loop.  Otherwise, the result is undefined.
 *
 * \param sock          The non-blocking socket to query.
 *
 * \returns a number greater than or equal to zero, indicating the number of
 * bytes in the read buffer.
 */
size_t ipc_socket_readbuffer_size(ipc_socket_context_t* sock)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != sock->impl);
    MODEL_ASSERT(NULL != ((ipc_socket_impl_t*)(sock->impl))->readbuf);

    /* get the socket impl. */
    ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

    /* we can't get a size on an invalid read buffer. */
    if (NULL == sock_impl->readbuf)
    {
        return 0;
    }

    /* get the number of bytes in the buffer. */
    return evbuffer_get_length(sock_impl->readbuf);
}
