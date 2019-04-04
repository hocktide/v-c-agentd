/**
 * \file protocolservice/unauthorized_protocol_service_event_loop.c
 *
 * \brief The event loop for the unauthorized protocol service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <signal.h>
#include <unistd.h>
#include <vpr/parameters.h>

/* forward decls */
static void unauthorized_protocol_service_ipc_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void unauthorized_protocol_service_ipc_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Event loop for the unauthorized protocol service.  This is the entry
 * point for the protocol service.  It handles the details of reacting to events
 * sent over the protocol service socket.
 *
 * \param protosock     The protocol service socket.  The protocol service
 *                      listens for connections on this socket.
 * \param logsock       The logging service socket.  The protocol service logs
 *                      on this socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *          attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if adding
 *            the protocol service socket to the event loop failed.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if running
 *            the protocol service event loop failed.
 */
int unauthorized_protocol_service_event_loop(
    int protosock, int UNUSED(logsock))
{
    int retval = 0;
    ipc_socket_context_t proto;
    ipc_event_loop_context_t loop;

    /* parameter sanity checking. */
    MODEL_ASSERT(protosock >= 0);
    MODEL_ASSERT(logsock >= 0);

    /* set the protocol socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS != ipc_make_noblock(protosock, &proto, NULL))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto done;
    }

    /* initialize an IPC event loop instance. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_init(&loop))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_protosock;
    }

    /* set the read, write, and error callbacks for the protocol socket. */
    ipc_set_readcb_noblock(&proto, &unauthorized_protocol_service_ipc_read);
    ipc_set_writecb_noblock(&proto, &unauthorized_protocol_service_ipc_write);

    /* on these signals, leave the event loop and shut down gracefully. */
    ipc_exit_loop_on_signal(&loop, SIGHUP);
    ipc_exit_loop_on_signal(&loop, SIGTERM);
    ipc_exit_loop_on_signal(&loop, SIGQUIT);

    /* add the protocol socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(&loop, &proto))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_ADD_FAILURE;
        goto cleanup_loop;
    }

    /* run the ipc event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_run(&loop))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_RUN_FAILURE;
        goto cleanup_loop;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_loop:
    dispose((disposable_t*)&loop);

cleanup_protosock:
    dispose((disposable_t*)&proto);

done:
    return retval;
}

/**
 * \brief Handle read events on the protocol socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this proto socket.
 */
static void unauthorized_protocol_service_ipc_read(
    ipc_socket_context_t* UNUSED(ctx), int UNUSED(event_flags),
    void* UNUSED(user_context))
{
}

/**
 * \brief Handle write events on the protocol socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this proto socket.
 */
static void unauthorized_protocol_service_ipc_write(
    ipc_socket_context_t* UNUSED(ctx), int UNUSED(event_flags),
    void* UNUSED(user_context))
{
}
