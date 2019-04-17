/**
 * \file ipc/ipc_sendsocket_block.c
 *
 * \brief Blocking write of a socket descriptor to a local peer.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Send a socket descriptor to the unix domain peer.
 *
 * On success, the socket sendsock is sent over the unix domain socket sock.
 * The caller maintains the local socket handle, and this should be closed by
 * the caller.
 *
 * \param sock          The unix domain socket through which sendsock should be
 *                      sent.
 * \param sendsock      The socket to send to the peer.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if this operation failed.
 */
int ipc_sendsocket_block(int sock, int sendsock)
{
    struct msghdr m;
    struct cmsghdr* cm;
    struct iovec iov;
    char buf[CMSG_SPACE(sizeof(int))];
    char dummy[2];

    /* parameter sanity check. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(sendsock >= 0);

    /* build message header. */
    memset(&m, 0, sizeof(m));
    m.msg_controllen = CMSG_SPACE(sizeof(int));
    m.msg_control = &buf;
    memset(m.msg_control, 0, m.msg_controllen);

    /* build socket control message header. */
    cm = CMSG_FIRSTHDR(&m);
    cm->cmsg_level = SOL_SOCKET;
    cm->cmsg_type = SCM_RIGHTS;
    cm->cmsg_len = CMSG_LEN(sizeof(int));
    memcpy(CMSG_DATA(cm), &sendsock, sizeof(int));
    m.msg_iov = &iov;
    m.msg_iovlen = 1;
    iov.iov_base = dummy;
    iov.iov_len = 1;
    memset(dummy, 0, sizeof(dummy));

    /* attempt to send this socket message to the peer. */
    if (sendmsg(sock, &m, 0) < 0)
    {
        return AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE;
    }

    /* success */
    return AGENTD_STATUS_SUCCESS;
}
