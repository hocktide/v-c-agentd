/**
 * \file ipc/ipc_make_block.c
 *
 * \brief Set a socket to blocking.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * \brief Set a socket for synchronous (blocking) I/O.  Afterward, the
 * ipc_*_block socket I/O methods can be used.
 *
 * On success, sd is synchronous, and all I/O on this socket will block.
 *
 * \param sd            The socket descriptor to make synchronous.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_FCNTL_GETFL_FAILURE if the fcntl flags could not be
 *        read.
 *      - AGENTD_ERROR_IPC_FCNTL_SETFL_FAILURE if the fcntl flags could not be
 *        updated.
 */
int ipc_make_block(int sock)
{
    MODEL_ASSERT(sock >= 0);

    /* get the flags for this socket. */
    int flags = fcntl(sock, F_GETFL);
    if (0 > flags)
        return AGENTD_ERROR_IPC_FCNTL_GETFL_FAILURE;

    /* clear the non-blocking bit. */
    flags &= ~O_NONBLOCK;

    /* set the flags for this socket. */
    if (0 > fcntl(sock, F_SETFL, flags))
        return AGENTD_ERROR_IPC_FCNTL_SETFL_FAILURE;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
