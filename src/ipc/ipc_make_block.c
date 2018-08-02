/**
 * \file ipc/ipc_make_block.c
 *
 * \brief Set a socket to blocking.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
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
 * \returns 0 on success and non-zero on failure.
 */
int ipc_make_block(int sock)
{
    /* get the flags for this socket. */
    int flags = fcntl(sock, F_GETFL);
    if (0 > flags)
        return 1;

    /* clear the non-blocking bit. */
    flags &= ~O_NONBLOCK;

    /* set the flags for this socket. */
    if (0 > fcntl(sock, F_SETFL, flags))
        return 2;

    /* success. */
    return 0;
}
