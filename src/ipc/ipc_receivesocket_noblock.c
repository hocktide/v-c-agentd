/**
 * \file ipc/ipc_receivesocket_noblock.c
 *
 * \brief Non-blocking read of a socket from the Unix domain socket.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/**
 * \brief Receive a socket descriptor from the unix domain peer.
 *
 * On success, the socket recvsock is received from the unix domain socket.
 * The caller owns the local socket handle recvsock and must close it when no
 * longer needed.
 *
 * \param ctx           The unix domain socket from which recvsock should be
 *                      received.
 * \param recvsock      The socket to receive from the peer.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if this operation would block.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if this operation failed.
 */
int ipc_receivesocket_noblock(ipc_socket_context_t* ctx, int* recvsock)
{
    struct msghdr m;
    struct cmsghdr* cm;
    struct iovec iov;
    char dummy[100];
    char buf[CMSG_SPACE(sizeof(int))];
    ssize_t readlen;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(NULL != ctx->impl);
    MODEL_ASSERT(NULL != recvsock);

    /* set up receive buffer */
    iov.iov_base = dummy;
    iov.iov_len = sizeof(dummy);
    memset(&m, 0, sizeof(m));
    m.msg_iov = &iov;
    m.msg_iovlen = 1;
    m.msg_controllen = CMSG_SPACE(sizeof(int));
    m.msg_control = buf;

    /* set sentry for the receive socket. */
    *recvsock = -1;

    /* read a message from the socket. */
    /* TODO - add support for handling partials, or require datagram socket. */
    readlen = recvmsg(ctx->fd, &m, MSG_WAITALL);
    if (readlen < 0)
    {
        if (EWOULDBLOCK == errno || EAGAIN == errno)
        {
            return AGENTD_ERROR_IPC_WOULD_BLOCK;
        }
        else
        {
            return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;
        }
    }

    /* read the socket control messages from the parent. */
    for (cm = CMSG_FIRSTHDR(&m); NULL != cm; cm = CMSG_NXTHDR(&m, cm))
    {
        /* if this is a socket message, save the socket. */
        if (SOL_SOCKET == cm->cmsg_level && SCM_RIGHTS == cm->cmsg_type)
        {
            memcpy(recvsock, CMSG_DATA(cm), sizeof(int));
            break;
        }
    }

    /* Verify that we received a socket. */
    if (*recvsock < 0)
    {
        return AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;
    }

    return AGENTD_STATUS_SUCCESS;
}
