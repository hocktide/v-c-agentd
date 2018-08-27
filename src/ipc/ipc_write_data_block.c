/**
 * \file ipc/ipc_write_data_block.c
 *
 * \brief Blocking write of a raw data packet to a socket
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Write a raw data packet.
 *
 * On success, the raw data packet value will be written, along with type
 * information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The raw data to write.
 * \param size          The size of the raw data to write.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_write_data_block(int sock, const void* val, uint32_t size)
{
    uint8_t typeval = IPC_DATA_TYPE_DATA_PACKET;

    /* attempt to write the type to the socket. */
    if (sizeof(typeval) != write(sock, &typeval, sizeof(typeval)))
        return 1;

    /* attempt to write the length of this data packet to the socket. */
    uint32_t hlen = htonl(size);
    if (sizeof(hlen) != write(sock, &hlen, sizeof(hlen)))
        return 2;

    /* attempt to write the data to the socket. */
    if (size != write(sock, val, size))
        return 3;

    /* success. */
    return 0;
}
