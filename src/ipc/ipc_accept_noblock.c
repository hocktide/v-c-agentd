/**
 * \file ipc/ipc_accept_noblock.c
 *
 * \brief Accept a socket from the non-blocking listening socket.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/**
 * \brief Accept a connection from a listen socket.
 *
 * On success, the socket specified by sock contains a connection to a remote
 * peer.  The address parameter contains data about the peer.
 *
 * \param ctx           The non-blocking socket context from which a connection
 *                      is accepted.
 * \param sock          A pointer to a socket descriptor that is populated with
 *                      the socket connection to the remote peer on success.
 * \param addr          A pointer to the buffer to hold the peer address.  This
 *                      is populated with the peer address on success.
 * \param addrsize      The value pointed to by addrsize should be set to the
 *                      maximum value of this buffer.  It is set to the number
 *                      of bytes used by the address on success.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if this operation would cause the socket
 *        to block.
 *      - AGENTD_ERROR_IPC_ACCEPT_NOBLOCK_FAILURE if accepting this socket
 *        failed.
 */
int ipc_accept_noblock(
    ipc_socket_context_t* ctx, int* sock, struct sockaddr* addr,
    socklen_t* addrsize)
{
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != addr);

    /* attempt to accept a value from the non-blocking listen socket. */
    int retval = accept(ctx->fd, addr, addrsize);
    if (retval < 0)
    {
        if (EAGAIN == errno || EWOULDBLOCK == errno)
        {
            return AGENTD_ERROR_IPC_WOULD_BLOCK;
        }
        /* Linuxisms: for TCP/IP, any of these means we should retry. */
        else if (
            ENETDOWN == errno || EPROTO == errno || ENOPROTOOPT == errno || EHOSTDOWN == errno || ENONET == errno || EHOSTUNREACH == errno || EOPNOTSUPP == errno || ENETUNREACH == errno || EINTR == errno)
        {
            return AGENTD_ERROR_IPC_ACCEPT_SHOULD_RETRY;
        }
        /* otherwise, the error can't be recovered from. */
        else
        {
            return AGENTD_ERROR_IPC_ACCEPT_NOBLOCK_FAILURE;
        }
    }

    /* success; set the socket and return success. */
    *sock = retval;
    return AGENTD_STATUS_SUCCESS;
}
