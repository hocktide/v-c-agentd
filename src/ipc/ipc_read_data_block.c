/**
 * \file ipc/ipc_read_data_block.c
 *
 * \brief Blocking read of a data packet value.
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
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int ipc_read_data_block(int sock, void** val, uint32_t* size)
{
    uint8_t type = 0U;
    uint32_t nsize = 0U;

    /* parameter sanity checks. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(NULL != val);
    MODEL_ASSERT(NULL != size);

    /* attempt to read the type info. */
    if (sizeof(type) != read(sock, &type, sizeof(type)))
        return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;

    /* verify that the type is IPC_DATA_TYPE_DATA_PACKET. */
    if (IPC_DATA_TYPE_DATA_PACKET != type)
        return AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE;

    /* attempt to read the size. */
    if (sizeof(nsize) != read(sock, &nsize, sizeof(nsize)))
        return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;

    /* convert the size to host byte order. */
    *size = ntohl(nsize);

    /* attempt to allocate memory for this data. */
    *val = malloc(*size);
    if (NULL == *val)
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;

    /* attempt to read the data. */
    if (*size != read(sock, *val, *size))
    {
        free(*val);
        *val = NULL;
        return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
