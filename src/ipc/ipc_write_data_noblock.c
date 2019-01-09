/**
 * \file ipc/ipc_write_data_noblock.c
 *
 * \brief Non-blocking write of a data packet value.
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
 * \brief Write a raw data packet to a non-blocking socket.
 *
 * On success, the raw data packet value will be written, along with type
 * information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The raw data to write.
 * \param size          The size of the raw data to write.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE if adding the type
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE if adding the size
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE if adding the
 *        payload data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_NONBLOCK_FAILURE if a non-blocking write
 *        failed.
 */
int ipc_write_data_noblock(
    ipc_socket_context_t* sock, const void* val, uint32_t size)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != sock->impl);
    MODEL_ASSERT(NULL != ((ipc_socket_impl_t*)sock->impl)->writebuf);
    MODEL_ASSERT(NULL != val);

    /* get the socket details. */
    ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

    /* attempt to write the type. */
    uint8_t type = IPC_DATA_TYPE_DATA_PACKET;
    if (0 != evbuffer_add(sock_impl->writebuf, &type, sizeof(type)))
    {
        return AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE;
    }

    /* attempt to write the size. */
    uint32_t nsize = htonl(size);
    if (0 != evbuffer_add(sock_impl->writebuf, &nsize, sizeof(nsize)))
    {
        return AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE;
    }

    /* add the data to the buffer. */
    if (0 != evbuffer_add(sock_impl->writebuf, val, size))
    {
        return AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE;
    }

    /* attempt to write the data. */
    int retval = ipc_socket_write_from_buffer(sock);
    if (retval < 0)
    {
        return AGENTD_ERROR_IPC_WRITE_NONBLOCK_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
