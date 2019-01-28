/**
 * \file ipc/ipc_event_loop_remove.c
 *
 * \brief Remove a non-blocking socket descriptor from an event loop.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/**
 * \brief Remove a non-blocking socket from the event loop.
 *
 * On success, the event loop will no longer manage events on this non-blocking
 * socket.  Note that the ownership for this socket context remains with the
 * caller.  It is the caller's responsibility to dispose the socket.
 *
 * \param loop          The event loop context from which this socket is
 *                      removed.
 * \param sock          This socket context is removed from the event loop.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_INVALID_ARGUMENT if the socket context does not
 *        belong to a loop.
 */
int ipc_event_loop_remove(
    ipc_event_loop_context_t* UNUSED(loop), ipc_socket_context_t* sock)
{
    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != loop);
    MODEL_ASSERT(NULL != sock);

    /* get the impls. */
    ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

    /* if the ev is not defined, then this socket was not assigned to a loop. */
    if (NULL == sock_impl->ev)
    {
        return AGENTD_ERROR_IPC_INVALID_ARGUMENT;
    }

    /* remove the event from the event loop. */
    event_free(sock_impl->ev);
    sock_impl->ev = NULL;

    /* free the buffers. */
    evbuffer_free(sock_impl->readbuf);
    evbuffer_free(sock_impl->writebuf);
    sock_impl->readbuf = sock_impl->writebuf = NULL;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
