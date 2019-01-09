/**
 * \file ipc/ipc_make_noblock.c
 *
 * \brief Set a socket to non-blocking and initialize a non-blocking socket
 * descriptor for use with non-blocking socket I/O.
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

/* forward decls */
static int ipc_fcntl_nonblock(int sock);
static void ipc_socket_context_dispose(void* disposable);

/**
 * \brief Set a socket for asynchronous (non-blocking) I/O.  Afterward, the
 * ipc_*_noblock socket I/O methods can be used.
 *
 * On success, sd is asynchronous, and all I/O on this socket will not block.
 * As such, all I/O should be done using \ref ipc_socket_context_t.
 * Furthermore, the ipc_socket_context_t structure is owned by the caller and
 * must be disposed using the dispose() method.
 *
 * \param sock          The socket descriptor to make asynchronous.
 * \param ctx           The socket context to initialize using this call.
 * \param user_context  The user context for this connection.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition
 *        occurred during this operation.
 *      - AGENTD_ERROR_IPC_FCNTL_GETFL_FAILURE if the fcntl flags could not be
 *        read.
 *      - AGENTD_ERROR_IPC_FCNTL_SETFL_FAILURE if the fcntl flags could not be
 *        updated.
 */
int ipc_make_noblock(
    int sock, ipc_socket_context_t* ctx, void* user_context)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(0 <= sock);
    MODEL_ASSERT(NULL != ctx);

    /* attempt to allocate an impl structure for this socket. */
    ipc_socket_impl_t* impl =
        (ipc_socket_impl_t*)malloc(sizeof(ipc_socket_impl_t));
    if (NULL == impl)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* clear this structure. */
    memset(impl, 0, sizeof(ipc_socket_impl_t));

    /* set the socket to non-blocking. */
    ssize_t retval = ipc_fcntl_nonblock(sock);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        free(impl);
        return retval;
    }

    /* set up the socket context. */
    memset(ctx, 0, sizeof(ipc_socket_context_t));
    ctx->hdr.dispose = &ipc_socket_context_dispose;
    ctx->fd = sock;
    ctx->user_context = user_context;
    ctx->impl = impl;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Set a socket to non-blocking using the OS fcntl mechanism.
 *
 * \param sock          The socket descriptor to make non-blocking.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int ipc_fcntl_nonblock(int sock)
{
    /* get the flags for this socket. */
    int flags = fcntl(sock, F_GETFL);
    if (0 > flags)
        return AGENTD_ERROR_IPC_FCNTL_GETFL_FAILURE;

    /* set the non-blocking bit. */
    flags |= O_NONBLOCK;

    /* set the flags for this socket. */
    if (0 > fcntl(sock, F_SETFL, flags))
        return AGENTD_ERROR_IPC_FCNTL_SETFL_FAILURE;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Dispose of a non-blocking socket context.
 *
 * \param disposable            The socket context to dispose.
 */
static void ipc_socket_context_dispose(void* disposable)
{
    ipc_socket_context_t* ctx = (ipc_socket_context_t*)disposable;
    ipc_socket_impl_t* impl = (ipc_socket_impl_t*)ctx->impl;

    /* sanity checks. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(NULL != impl);

    /* if an event is set for this socket, free it and the read/write buffers */
    if (impl->ev)
    {
        event_free(impl->ev);
        evbuffer_free(impl->readbuf);
        evbuffer_free(impl->writebuf);
    }

    /* free the impl. */
    free(impl);

    /* clear the structure. */
    memset(ctx, 0, sizeof(ipc_socket_context_t));
}
