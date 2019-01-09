/**
 * \file ipc/ipc_socketpair.c
 *
 * \brief Create a socketpair for inter-process communication.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * \brief Create a socket pair of the given type and protocol for the given
 * domain.
 *
 * On success, lhs and rhs are set to the left-hand and right-hand sides of the
 * socket pair.
 *
 * \param domain        The domain for the socket.
 * \param type          The type of socket.
 * \param protocol      The protocol for the socket. 
 * \param lhs           Pointer to the integer variable updated to the
 *                      left-hand-side descriptor.
 * \param rhs           Pointer to the integer variable updated to the
 *                      right-hand-side descriptor for the socket pair.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_SOCKETPAIR_FAILURE if creating the socket pair
 *        failed.
 */
int ipc_socketpair(int domain, int type, int protocol, int* lhs, int* rhs)
{
    int sd[2];

    MODEL_ASSERT(NULL != lhs);
    MODEL_ASSERT(NULL != rhs);

    /* build the socket pair. */
    if (0 != socketpair(domain, type, protocol, sd))
    {
        return AGENTD_ERROR_IPC_SOCKETPAIR_FAILURE;
    }

    /* assign the socket descriptors. */
    *lhs = sd[0];
    *rhs = sd[1];

    return AGENTD_STATUS_SUCCESS;
}
