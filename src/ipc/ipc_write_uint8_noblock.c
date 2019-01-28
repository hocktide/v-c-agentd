/**
 * \file ipc/ipc_write_uint8_noblock.c
 *
 * \brief Non-blocking write of a uint8 value.
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
 * \brief Write a uint8_t value to a non-blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE if adding the type
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE if adding the size
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE if adding the
 *        payload data to the write buffer failed.
 */
int ipc_write_uint8_noblock(ipc_socket_context_t* sock, uint8_t val)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != sock->impl);
    MODEL_ASSERT(NULL != ((ipc_socket_impl_t*)sock->impl)->writebuf);

    /* get the socket details. */
    ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

    /* attempt to write the type. */
    uint8_t type = IPC_DATA_TYPE_UINT8;
    if (0 != evbuffer_add(sock_impl->writebuf, &type, sizeof(type)))
    {
        return AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE;
    }

    /* attempt to write the size. */
    uint32_t nsize = htonl(sizeof(val));
    if (0 != evbuffer_add(sock_impl->writebuf, &nsize, sizeof(nsize)))
    {
        return AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE;
    }

    /* attempt to write the data. */
    uint8_t oval = htonll(val);
    if (0 != evbuffer_add(sock_impl->writebuf, &oval, sizeof(oval)))
    {
        return AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
