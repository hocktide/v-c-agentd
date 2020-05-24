/**
 * \file protocolservice/unauthorized_protocol_service_connection_read.c
 *
 * \brief Read from a protocol connection socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Read data from a connection
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this proto socket.
 */
void unauthorized_protocol_service_connection_read(
    ipc_socket_context_t* UNUSED(ctx), int UNUSED(event_flags),
    void* user_context)
{
    /* get the instance from the user context. */
    unauthorized_protocol_connection_t* conn =
        (unauthorized_protocol_connection_t*)user_context;

    /* don't read anything from this socket if we're shutting down. */
    if (conn->svc->force_exit)
        return;

    /* dispatch based on the current connection state. */
    switch (conn->state)
    {
        /* we expect to read a handshake request from the client. */
        case UPCS_READ_HANDSHAKE_REQ_FROM_CLIENT:
            unauthorized_protocol_service_connection_handshake_req_read(conn);
            break;

        /* we expect to read a handshake acknowledgement from the client. */
        case UPCS_READ_HANDSHAKE_ACK_FROM_CLIENT:
            unauthorized_protocol_service_connection_handshake_ack_read(conn);
            break;

        /* we expect to read a command request from the client. */
        case APCS_READ_COMMAND_REQ_FROM_CLIENT:
            unauthorized_protocol_service_command_read(conn);
            break;

        /* we are not currently expecting input, so wait until we enter a state
         * that expects input. */
        default:
            break;
    }
}
