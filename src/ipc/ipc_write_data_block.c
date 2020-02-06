/**
 * \file ipc/ipc_write_data_block.c
 *
 * \brief Blocking write of a raw data packet to a socket
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
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
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if writing data failed.
 */
int ipc_write_data_block(int sock, const void* val, uint32_t size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(NULL != val);

    uint8_t typeval = IPC_DATA_TYPE_DATA_PACKET;

    /* attempt to write the type to the socket. */
    if (sizeof(typeval) != write(sock, &typeval, sizeof(typeval)))
        return AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;

    /* attempt to write the length of this data packet to the socket. */
    uint32_t hlen = htonl(size);
    if (sizeof(hlen) != write(sock, &hlen, sizeof(hlen)))
        return AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;

    /* attempt to write the data to the socket. */
    if (size != write(sock, val, size))
        return AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
