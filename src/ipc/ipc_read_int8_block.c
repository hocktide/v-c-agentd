/**
 * \file ipc/ipc_read_int8_block.c
 *
 * \brief Blocking read of a int8_t value.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <agentd/inet.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Read an int8_t value from the blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if the data size read from
 *        the socket was unexpected.
 */
int ipc_read_int8_block(int sock, int8_t* val)
{
    uint8_t type = 0U;
    uint32_t nsize = 0U;
    uint32_t size = 0U;

    /* attempt to read the type info. */
    if (sizeof(type) != read(sock, &type, sizeof(type)))
        return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;

    /* verify that the type is IPC_DATA_TYPE_INT8. */
    if (IPC_DATA_TYPE_INT8 != type)
        return AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE;

    /* attempt to read the size. */
    if (sizeof(nsize) != read(sock, &nsize, sizeof(nsize)))
        return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;

    /* convert the size to host byte order. */
    size = ntohl(nsize);

    /* verify the size. */
    if (size != sizeof(int8_t))
        return AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE;

    /* attempt to read the value. */
    if (size != read(sock, val, sizeof(int8_t)))
        return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
