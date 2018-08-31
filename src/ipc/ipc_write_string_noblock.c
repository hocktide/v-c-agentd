/**
 * \file ipc/ipc_write_string_noblock.c
 *
 * \brief Non-blocking write of a string value.
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
 * \brief Write a character string to a non-blocking socket.
 *
 * On success, the character string value is written, along with type
 * information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The string to write.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_write_string_noblock(ipc_socket_context_t* sock, const char* val)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != sock->impl);
    MODEL_ASSERT(NULL != ((ipc_socket_impl_t*)sock->impl)->writebuf);
    MODEL_ASSERT(NULL != val);

    /* get the socket details. */
    ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

    /* attempt to write the type. */
    uint8_t type = IPC_DATA_TYPE_STRING;
    if (0 != evbuffer_add(sock_impl->writebuf, &type, sizeof(type)))
    {
        return 1;
    }

    /* attempt to write the size. */
    uint32_t size = strlen(val);
    uint32_t nsize = htonl(size);
    if (0 != evbuffer_add(sock_impl->writebuf, &nsize, sizeof(nsize)))
    {
        return 2;
    }

    /* attempt to write the string. */
    if (0 != evbuffer_add(sock_impl->writebuf, val, size))
    {
        return 3;
    }

    /* success. */
    return 0;
}
