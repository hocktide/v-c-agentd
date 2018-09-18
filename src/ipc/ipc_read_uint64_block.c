/**
 * \file ipc/ipc_read_uint64_block.c
 *
 * \brief Blocking read of a uint64_t value.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/inet.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Read a uint64_t value from the blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_read_uint64_block(int sock, uint64_t* val)
{
    uint8_t type = 0U;
    uint32_t nsize = 0U;
    uint32_t size = 0U;
    uint64_t nval = 0U;

    /* attempt to read the type info. */
    if (sizeof(type) != read(sock, &type, sizeof(type)))
        return 1;

    /* verify that the type is IPC_DATA_TYPE_UINT64. */
    if (IPC_DATA_TYPE_UINT64 != type)
        return 2;

    /* attempt to read the size. */
    if (sizeof(nsize) != read(sock, &nsize, sizeof(nsize)))
        return 3;

    /* convert the size to host byte order. */
    size = ntohl(nsize);

    /* verify the size. */
    if (size != sizeof(uint64_t))
        return 4;

    /* attempt to read the value. */
    if (size != read(sock, &nval, sizeof(uint64_t)))
        return 5;

    /* convert this value to host byte order. */
    *val = ntohll(nval);

    /* success. */
    return 0;
}
