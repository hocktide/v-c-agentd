/**
 * \file ipc/ipc_write_int64_block.c
 *
 * \brief Blocking write of a uint64_t value.
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

/**
 * \brief Write an int64_t value to the blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if writing data failed.
 */
int ipc_write_int64_block(int sock, int64_t val)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(sock >= 0);

    uint8_t typeval = IPC_DATA_TYPE_INT64;

    /* attempt to write the type to the socket. */
    if (sizeof(typeval) != write(sock, &typeval, sizeof(typeval)))
        return AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;

    /* attempt to write the length of this value to the socket. */
    uint32_t hlen = htonl(sizeof(val));
    if (sizeof(hlen) != write(sock, &hlen, sizeof(hlen)))
        return AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;

    /* attempt to write the value to the socket. */
    int64_t oval = htonll(val);
    if (sizeof(oval) != write(sock, &oval, sizeof(oval)))
        return AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
