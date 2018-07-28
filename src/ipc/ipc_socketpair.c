/**
 * \file ipc/ipc_socketpair.c
 *
 * \brief Create a socketpair for inter-process communication.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
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
 */
int ipc_socketpair(int domain, int type, int protocol, int* lhs, int* rhs)
{
    int retval;
    int sd[2];

    MODEL_ASSERT(NULL != lhs);
    MODEL_ASSERT(NULL != rhs);

    /* build the socket pair. */
    retval = socketpair(domain, type, protocol, sd);
    if (0 != retval)
    {
        return retval;
    }

    /* assign the socket descriptors. */
    *lhs = sd[0];
    *rhs = sd[1];

    return 0;
}
