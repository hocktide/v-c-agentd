/**
 * \file ipc/ipc_read_data_nblock.c
 *
 * \brief Non-blocking read of a data packet value.
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
 * \brief Read a raw data packet from a non-blocking socket.
 *
 * On success, a raw data buffer is allocated and read, along with type
 * information and size.  The caller owns this buffer and is responsible for
 * freeing it when it is no longer in use.
 *
 * \param sd            The socket descriptor from which the value is read.
 * \param val           Pointer to the pointer of the raw data buffer.
 * \param size          Pointer to the variable to receive the size of this
 *                      packet.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_read_data_noblock(
    ipc_socket_context_t* sock, void** val, uint32_t* size)
{
    ssize_t retval = 0;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != sock->impl);
    MODEL_ASSERT(NULL != ((ipc_socket_impl_t*)sock->impl)->readbuf);
    MODEL_ASSERT(NULL != val);
    MODEL_ASSERT(NULL != size);

    /* get the socket details. */
    ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

    /* compute the header size. */
    ssize_t header_sz = sizeof(uint8_t) + sizeof(uint32_t);

    /* we need the header data. */
    uint8_t* mem = (uint8_t*)evbuffer_pullup(sock_impl->readbuf, header_sz);
    if (NULL == mem)
    {
        retval = IPC_ERROR_CODE_WOULD_BLOCK;
        goto done;
    }

    /* if the type does not match our expected type, return an error. */
    if (IPC_DATA_TYPE_DATA_PACKET != mem[0])
    {
        retval = 1;
        goto done;
    }

    /* decode the size of this packet. */
    uint32_t nsize = 0;
    memcpy(&nsize, mem + 1, sizeof(uint32_t));

    /* sanity check on size. */
    *size = ntohl(nsize);
    if (*size <= 0 || *size >= 1024 * 1024 * 1024)
    {
        retval = 2;
        goto done;
    }

    /* if the buffer size is less than this size, wait for more data to be
     * available. */
    if (evbuffer_get_length(sock_impl->readbuf) < (*size + (size_t)header_sz))
    {
        retval = IPC_ERROR_CODE_WOULD_BLOCK;
        goto done;
    }

    /* allocate memory for this packet. */
    *val = malloc(*size);
    if (NULL == *val)
    {
        retval = 3;
        goto done;
    }

    /* drain the header from the buffer. */
    if (header_sz != evbuffer_drain(sock_impl->readbuf, header_sz))
    {
        retval = 4;
        goto cleanup_val;
    }

    /* read the data from the buffer. */
    if (*size != (size_t)evbuffer_remove(sock_impl->readbuf, *val, *size))
    {
        retval = 5;
        goto cleanup_val;
    }

    /* success. */
    retval = 0;
    goto done;

cleanup_val:
    free(*val);
    *val = NULL;

done:
    return retval;
}
