/**
 * \file ipc/ipc_read_string_block.c
 *
 * \brief Blocking read of a string value.
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

/**
 * \brief Read a character string from the blocking socket.
 *
 * On success, a character string value is allocated and read, along with type
 * information and size.  The caller owns this character string and is
 * responsible for freeing it when it is no longer in use.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to the string pointer to hold the string value
 *                      on success.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_read_string_block(int sock, char** val)
{
    uint8_t type = 0U;
    uint32_t nsize = 0U;
    uint32_t size = 0U;

    /* attempt to read the type info. */
    if (sizeof(type) != read(sock, &type, sizeof(type)))
        return 1;

    /* verify that the type is IPC_DATA_TYPE_STRING. */
    if (IPC_DATA_TYPE_STRING != type)
        return 2;

    /* attempt to read the size. */
    if (sizeof(nsize) != read(sock, &nsize, sizeof(nsize)))
        return 3;

    /* convert the size to host byte order. */
    size = ntohl(nsize);

    /* attempt to allocate memory for this string. */
    *val = (char*)malloc(size + 1);

    /* attempt to read the string. */
    if (size != read(sock, *val, size))
    {
        free(*val);
        *val = NULL;
        return 4;
    }

    /* set the asciiz. */
    (*val)[size] = 0;

    /* success. */
    return 0;
}
