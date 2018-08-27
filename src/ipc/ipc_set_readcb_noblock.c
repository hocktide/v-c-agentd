/**
 * \file ipc/ipc_set_readcb_noblock.c
 *
 * \brief Set the read callback for this non-blocking socket.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "ipc_internal.h"

/**
 * \brief Set the read event callback for a non-blocking socket.
 *
 * \note This method can only be called BEFORE a socket has been added to the
 * event loop.  Otherwise, the callback will not be properly set.
 *
 * \param sock          The socket to set.
 * \param cb            The callback to set.
 */
void ipc_set_readcb_noblock(
    ipc_socket_context_t* sock, ipc_socket_event_cb_t cb)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != cb);

    /* assign the read callback. */
    sock->read = cb;
}
