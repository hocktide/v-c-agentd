/**
 * \file listenservice/listenservice_ipc_accept.c
 *
 * \brief Accept a socket from the given listen socket.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "listenservice_internal.h"

/**
 * \brief Read callback on listen sockets to accept a new socket.
 *
 * This callback is registered as part of the ipc callback mechanism for a
 * listen socket.  It forwards a socket to the accept socket in the \ref
 * listenservice_instance_t context structure.
 */
void listenservice_ipc_accept(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    ssize_t retval = 0;
    int sock = 0;
    struct sockaddr_in peer;
    socklen_t peersize = sizeof(peer);
    listenservice_instance_t* instance =
        (listenservice_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_READ);
    MODEL_ASSERT(NULL != instance);

    /* don't accept new connections from this socket if we are quiescing. */
    if (instance->listenservice_force_exit)
        return;

    /* attempt to accept a socket. */
    retval =
        ipc_accept_noblock(ctx, &sock, (struct sockaddr*)&peer, &peersize);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK == retval || AGENTD_ERROR_IPC_ACCEPT_SHOULD_RETRY == retval)
    {
        return;
    }
    else if (AGENTD_STATUS_SUCCESS != retval)
    {
        listenservice_exit_event_loop(instance);
        return;
    }

    /* attempt to send this socket to the protocol service. */
    retval =
        ipc_sendsocket_block(instance->acceptsock, sock);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        listenservice_exit_event_loop(instance);
        goto cleanup_socket;
    }

cleanup_socket:
    close(sock);
}
