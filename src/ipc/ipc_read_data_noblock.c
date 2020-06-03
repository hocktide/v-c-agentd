/**
 * \file ipc/ipc_read_data_nblock.c
 *
 * \brief Non-blocking read of a data packet value.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
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
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if the the operation was halted because
 *        it would block this thread.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if an unexpected data type
 *        was encountered when attempting to read this value.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if an unexpected data size
 *        was encountered when attempting to read this value.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error condition while executing.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE if draining the read buffer
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE if removing the payload
 *        from the read buffer failed.
 */
int ipc_read_data_noblock(
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

    /* read data from the socket into our buffer. */
    retval = evbuffer_read(sock_impl->readbuf, sock->fd, -1);
    if (retval < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
    {
        retval = AGENTD_ERROR_IPC_WOULD_BLOCK;
        /* fall through, since we might have enough data in the buffer. */
    }
    else if (retval < 0)
    {
        retval = AGENTD_ERROR_IPC_EVBUFFER_READ_FAILURE;
        goto done;
    }
    else if (retval == 0)
    {
        retval = AGENTD_ERROR_IPC_EVBUFFER_EOF;
        goto done;
    }

    /* we need the header data. */
    uint8_t* mem = (uint8_t*)evbuffer_pullup(sock_impl->readbuf, header_sz);
    if (NULL == mem)
    {
        retval = AGENTD_ERROR_IPC_WOULD_BLOCK;
        goto done;
    }

    /* if the type does not match our expected type, return an error. */
    if (IPC_DATA_TYPE_DATA_PACKET != mem[0])
    {
        retval = AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE;
        goto done;
    }

    /* decode the size of this packet. */
    uint32_t nsize = 0;
    memcpy(&nsize, mem + 1, sizeof(uint32_t));

    /* sanity check on size. */
    *size = ntohl(nsize);
    if (*size <= 0 || *size >= 1024 * 1024 * 1024)
    {
        retval = AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE;
        goto done;
    }

    /* if the buffer size is less than this size, wait for more data to be
     * available. */
    if (evbuffer_get_length(sock_impl->readbuf) < (*size + (size_t)header_sz))
    {
        retval = AGENTD_ERROR_IPC_WOULD_BLOCK;
        goto done;
    }

    /* allocate memory for this packet. */
    *val = malloc(*size);
    if (NULL == *val)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* drain the header from the buffer. */
    if (0 != evbuffer_drain(sock_impl->readbuf, header_sz))
    {
        retval = AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE;
        goto cleanup_val;
    }

    /* read the data from the buffer. */
    if (*size != (size_t)evbuffer_remove(sock_impl->readbuf, *val, *size))
    {
        retval = AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE;
        goto cleanup_val;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

cleanup_val:
    free(*val);
    *val = NULL;

done:
    return retval;
}
