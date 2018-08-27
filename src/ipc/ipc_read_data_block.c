/**
 * \file ipc/ipc_read_data_block.c
 *
 * \brief Blocking read of a data packet value.
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
 * \brief Read a raw data packet from the blocking socket.
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
ssize_t ipc_read_data_block(int sock, void** val, uint32_t* size)
{
    uint8_t type = 0U;
    uint32_t nsize = 0U;

    /* attempt to read the type info. */
    if (sizeof(type) != read(sock, &type, sizeof(type)))
        return 1;

    /* verify that the type is IPC_DATA_TYPE_DATA_PACKET. */
    if (IPC_DATA_TYPE_DATA_PACKET != type)
        return 2;

    /* attempt to read the size. */
    if (sizeof(nsize) != read(sock, &nsize, sizeof(nsize)))
        return 3;

    /* convert the size to host byte order. */
    *size = ntohl(nsize);

    /* attempt to allocate memory for this data. */
    *val = malloc(*size);

    /* attempt to read the data. */
    if (*size != read(sock, *val, *size))
    {
        free(*val);
        *val = NULL;
        return 4;
    }

    /* success. */
    return 0;
}
