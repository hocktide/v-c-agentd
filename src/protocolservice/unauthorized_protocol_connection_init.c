/**
 * \file protocolservice/unauthorized_protocol_connection_init.c
 *
 * \brief Initialize an unauthorized protocol connection.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/* forward decls. */
static void unauthorized_protocol_connection_dispose(void* disposable);

/**
 * \brief Initialize an unauthorized protocol connection instance.
 *
 * This instance takes ownership of the socket, which will be closed when this
 * instance is disposed.  This is different than the default behavior of
 * ipc_make_noblock.
 *
 * \param conn          The connection to initialize.
 * \param sock          The socket descriptor for this instance.
 * \param svc           The unauthorized protocol service instance.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE if a failure
 *        occurred when attempting to make a non-blocking socket.
 */
int unauthorized_protocol_connection_init(
    unauthorized_protocol_connection_t* conn, int sock,
    unauthorized_protocol_service_instance_t* svc)
{
    int retval = AGENTD_STATUS_SUCCESS;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != conn);
    MODEL_ASSERT(sock > 0);

    /* clear the connection. */
    memset(conn, 0, sizeof(unauthorized_protocol_connection_t));

    /* initialize the data for this instance. */
    conn->hdr.dispose = &unauthorized_protocol_connection_dispose;
    conn->state = UPCS_READ_HANDSHAKE_REQ_FROM_CLIENT;
    conn->svc = svc;

    /* attempt to make this socket non-blocking. */
    retval = ipc_make_noblock(sock, &conn->ctx, conn);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        return AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
    }

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Dispose of a connection instance.
 *
 * \param disposable        The instance to dispose.
 */
static void unauthorized_protocol_connection_dispose(void* disposable)
{
    unauthorized_protocol_connection_t* conn =
        (unauthorized_protocol_connection_t*)disposable;

    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != conn);

    /* keep a copy of the socket. */
    int sock = conn->ctx.fd;

    /* dispose of the socket context. */
    dispose((disposable_t*)&conn->ctx);

    /* close the socket. */
    close(sock);

    /* clean up the instance. */
    memset(conn, 0, sizeof(unauthorized_protocol_connection_t));
}
