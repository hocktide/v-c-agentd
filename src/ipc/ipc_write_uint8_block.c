/**
 * \file ipc/ipc_write_uint8_block.c
 *
 * \brief Blocking write of a uint8_t value.
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
 * \brief Write a uint8_t value to the blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_write_uint8_block(int sock, uint8_t val)
{
    uint8_t typeval = IPC_DATA_TYPE_UINT8;

    /* attempt to write the type to the socket. */
    if (sizeof(typeval) != write(sock, &typeval, sizeof(typeval)))
        return 1;

    /* attempt to write the length of this value to the socket. */
    uint32_t hlen = htonl(sizeof(val));
    if (sizeof(hlen) != write(sock, &hlen, sizeof(hlen)))
        return 2;

    /* attempt to write the value to the socket. */
    if (sizeof(val) != write(sock, &val, sizeof(val)))
        return 3;

    /* success. */
    return 0;
}
