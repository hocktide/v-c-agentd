/**
 * \file protocolservice/unauthorized_protocol_service_connection_write.c
 *
 * \brief Write to a protocol connection socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <errno.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief The write callback for managing writes to the client connection and
 * for advancing the state machine after the write is completed.
 *
 * \param ctx           The socket context for this write callback.
 * \param event_flags   The event flags that led to this callback being called.
 * \param user_context  The user context for this callback (expected: a protocol
 *                      connection).
 */
void unauthorized_protocol_service_connection_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    /* get the instance from the user context. */
    unauthorized_protocol_connection_t* conn =
        (unauthorized_protocol_connection_t*)user_context;

    /* no need to continue if we're shutting down. */
    if (conn->svc->force_exit)
        return;

    /* first, see if we still need to write data. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        /* attempt to write data. */
        int bytes_written = ipc_socket_write_from_buffer(ctx);

        /* was the socket closed, or was there an error? */
        if (bytes_written == 0 || (bytes_written < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)))
        {
            unauthorized_protocol_service_close_connection(conn);
            return;
        }
        /* force the write callback to happen again. */
        else
        {
            ipc_set_writecb_noblock(
                &conn->ctx, &unauthorized_protocol_service_connection_write,
                &conn->svc->loop);
        }
    }
    else
    {
        /* we are done writing to this socket. */
        ipc_set_writecb_noblock(
            &conn->ctx, NULL, &conn->svc->loop);

        /* should we change state? */
        switch (conn->state)
        {
            /* after writing the handshake response to the client, read the
             * ack. */
            case UPCS_WRITE_HANDSHAKE_RESP_TO_CLIENT:
                conn->state = UPCS_READ_HANDSHAKE_ACK_FROM_CLIENT;
                ipc_set_readcb_noblock(
                    &conn->ctx, &unauthorized_protocol_service_connection_read,
                    &conn->svc->loop);
                return;

            /* after writing the handshake ack response to the client, set the
             * state to authorized. */
            case UPCS_WRITE_HANDSHAKE_ACK_TO_CLIENT:
                unauthorized_protocol_service_dataservice_request_child_context(
                    conn);
                return;

            /* after writing a command response to the client, reset. */
            case APCS_WRITE_COMMAND_RESP_TO_CLIENT:
                conn->state = APCS_READ_COMMAND_REQ_FROM_CLIENT;
                ipc_set_readcb_noblock(
                    &conn->ctx, &unauthorized_protocol_service_connection_read,
                    &conn->svc->loop);
                return;

            /* if we are in a forced unauthorized state, close the
             * connection. */
            case UPCS_UNAUTHORIZED:
                unauthorized_protocol_service_close_connection(conn);
                return;

            /* unexpected state.  Close connection. */
            default:
                unauthorized_protocol_service_close_connection(conn);
                return;
        }
    }
}
