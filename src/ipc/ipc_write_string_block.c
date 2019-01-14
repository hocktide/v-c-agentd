/**
 * \file ipc/ipc_write_string_block.c
 *
 * \brief Blocking write of a string to a socket
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
 * \brief Write a character string to the blocking socket.
 *
 * On success, the character string value is written, along with type
 * information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The string to write.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if writing data failed.
 */
int ipc_write_string_block(int sock, const char* val)
{
    uint8_t typeval = IPC_DATA_TYPE_STRING;

    /* attempt to write the type to the socket. */
    if (sizeof(typeval) != write(sock, &typeval, sizeof(typeval)))
        return AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;

    /* attempt to write the length of this string to the socket. */
    uint32_t len = strlen(val);
    uint32_t hlen = htonl(len);
    if (sizeof(hlen) != write(sock, &hlen, sizeof(hlen)))
        return AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;

    /* attempt to write the string to the socket. */
    if (len != write(sock, val, len))
        return AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
