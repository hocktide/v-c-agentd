/**
 * \file ipc/ipc_read_uint8_noblock.c
 *
 * \brief Non-blocking read of a uint8 value.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/inet.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/**
 * \brief Read a uint8_t value from a non-blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_read_uint8_noblock(ipc_socket_context_t* sock, uint8_t* val)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != sock->impl);
    MODEL_ASSERT(NULL != ((ipc_socket_impl_t*)sock->impl)->readbuf);
    MODEL_ASSERT(NULL != val);

    /* get the socket details. */
    ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

    /* compute the header size. */
    ssize_t header_sz = sizeof(uint8_t) + sizeof(uint32_t);

    /* we need the header data. */
    uint8_t* mem = (uint8_t*)evbuffer_pullup(sock_impl->readbuf, header_sz);
    if (NULL == mem)
    {
        return IPC_ERROR_CODE_WOULD_BLOCK;
    }

    /* if the type does not match our expected type, return an error. */
    if (IPC_DATA_TYPE_UINT8 != mem[0])
    {
        return 1;
    }

    /* decode the size of this packet. */
    uint32_t nsize = 0;
    memcpy(&nsize, mem + 1, sizeof(uint32_t));

    /* sanity check on size. */
    uint32_t size = ntohl(nsize);
    if (size != sizeof(uint8_t))
    {
        return 2;
    }

    /* if the buffer size is less than this size, wait for more data to be
     * available. */
    if (evbuffer_get_length(sock_impl->readbuf) < (size + (size_t)header_sz))
    {
        return IPC_ERROR_CODE_WOULD_BLOCK;
    }

    /* drain the header from the buffer. */
    if (header_sz != evbuffer_drain(sock_impl->readbuf, header_sz))
    {
        return 3;
    }

    /* read the data from the buffer. */
    if (size != (size_t)evbuffer_remove(sock_impl->readbuf, val, size))
    {
        return 4;
    }

    /* success. */
    return 0;
}
