/**
 * \file protocolservice/consensus_service_event_loop.c
 *
 * \brief The event loop for the consensus service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <agentd/consensusservice.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <errno.h>
#include <signal.h>
#include <vpr/parameters.h>

#include "consensusservice_internal.h"

/* forward decls */
static void consensus_service_control_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void consensus_service_control_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void consensus_service_data_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Event loop for the consensus service.  This is the entry point for the
 * consensus service.
 *
 * \param datasock      The data service socket.  The protocol service
 *                      communicates with the dataservice using this socket.
 * \param logsock       The logging service socket.  The protocol service logs
 *                      on this socket.
 * \param controlsock   The socket used to control the consensus service.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_CONSENSUSSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *            attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if adding
 *            the protocol service socket to the event loop failed.
 *          - AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if
 *            running the protocol service event loop failed.
 */
int consensus_service_event_loop(
    int datasock, int UNUSED(logsock), int controlsock)
{
    int retval = AGENTD_STATUS_SUCCESS;
    consensusservice_instance_t* instance = NULL;
    ipc_socket_context_t data;
    ipc_socket_context_t control;
    ipc_event_loop_context_t loop;

    /* parameter sanity checking. */
    MODEL_ASSERT(datasock >= 0);
    MODEL_ASSERT(logsock >= 0);
    MODEL_ASSERT(controlsock >= 0);

    /* Create the consensus service instance. */
    instance = consensusservice_instance_create();
    if (NULL == instance)
    {
        retval = AGENTD_ERROR_CONSENSUSSERVICE_INSTANCE_CREATE_FAILURE;
        goto done;
    }

    /* set the control socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_make_noblock(controlsock, &control, instance))
    {
        retval = AGENTD_ERROR_CONSENSUSSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_instance;
    }

    /* set the data socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_make_noblock(datasock, &data, instance))
    {
        retval = AGENTD_ERROR_CONSENSUSSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_control_socket;
    }

    /* initialize the IPC event loop instance. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_init(&loop))
    {
        retval = AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_INIT;
        goto cleanup_data_socket;
    }

    /* set a reference to the event loop in the instance. */
    instance->loop_context = &loop;

    /* set the read callback on the data and control sockets. */
    ipc_set_readcb_noblock(&control, &consensus_service_control_read, NULL);
    ipc_set_readcb_noblock(&data, &consensus_service_data_read, NULL);

    /* on these signals, leave the event loop and shut down gracefully. */
    ipc_exit_loop_on_signal(&loop, SIGHUP);
    ipc_exit_loop_on_signal(&loop, SIGTERM);
    ipc_exit_loop_on_signal(&loop, SIGQUIT);

    /* add the control socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(&loop, &control))
    {
        retval = AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_ADD_FAILURE;
        goto cleanup_loop;
    }

    /* add the data socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(&loop, &data))
    {
        retval = AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_ADD_FAILURE;
        goto cleanup_loop;
    }

    /* run the ipc event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_run(&loop))
    {
        retval = AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_RUN_FAILURE;
        goto cleanup_loop;
    }

cleanup_loop:
    dispose((disposable_t*)&loop);

cleanup_data_socket:
    dispose((disposable_t*)&data);

cleanup_control_socket:
    dispose((disposable_t*)&control);

cleanup_instance:
    if (NULL != instance)
    {
        dispose((disposable_t*)instance);
        free(instance);
    }

done:
    return retval;
}

/**
 * \brief Handle read events on the control socket.
 *
 * \param ctx               The non-blocking socket context.
 * \param event_flags       The event that triggered this callback.
 * \param user_context      The user context for this control socket.
 */
static void consensus_service_control_read(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    int retval = 0;
    void* req;
    uint32_t size = 0;
    consensusservice_instance_t* instance =
        (consensusservice_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_READ);
    MODEL_ASSERT(NULL != instance);

    /* don't process data from this socket if we have been forced to exit. */
    if (instance->force_exit)
        return;

    /* attempt to read a request. */
    retval = ipc_read_data_noblock(ctx, &req, &size);
    switch (retval)
    {
        /* on success, decode and dispatch. */
        case AGENTD_STATUS_SUCCESS:
            if (AGENTD_STATUS_SUCCESS !=
                consensus_service_decode_and_dispatch_control_command(
                    instance, ctx, req, size))
            {
                /* a bad control message means we should shut down. */
                instance->force_exit = true;
                ipc_exit_loop(instance->loop_context);
            }

            /* clear and free the request data. */
            memset(req, 0, size);
            free(req);
            break;

        /* wait for more data on the socket. */
        case AGENTD_ERROR_IPC_WOULD_BLOCK:
            break;

        /* any other error code indicates that we should no longer trust the
         * control socket. */
        default:
            instance->force_exit = true;
            ipc_exit_loop(instance->loop_context);
            break;
    }

    /* fire up the write callback if there is data to write. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        ipc_set_writecb_noblock(
            ctx, &consensus_service_control_write, instance->loop_context);
    }
}

/**
 * \brief Handle read events on the data socket.
 *
 * \param ctx               The non-blocking socket context.
 * \param event_flags       The event that triggered this callback.
 * \param user_context      The user context for this control socket.
 */
static void consensus_service_data_read(
    ipc_socket_context_t* UNUSED(ctx), int UNUSED(event_flags),
    void* UNUSED(user_context))
{
    /* TODO - implement. */
}

/**
 * \brief Handle write events on the data socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this data socket.
 */
static void consensus_service_control_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    consensusservice_instance_t* instance =
        (consensusservice_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_WRITE);
    MODEL_ASSERT(NULL != instance);

    /* write data if there is data to write. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        /* attempt to write data. */
        int bytes_written = ipc_socket_write_from_buffer(ctx);

        /* has the socket been closed? */
        if (0 == bytes_written)
        {
            goto exit_failure;
        }
        /* has there been a socket error? */
        else if (bytes_written < 0)
        {
            /* for any error except retrying, exit the loop. */
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                goto exit_failure;
            }
        }
    }
    else
    {
        /* disable the callback if there is no more data to write. */
        ipc_set_writecb_noblock(
            ctx, NULL, instance->loop_context);
    }

    /* success. */
    return;

exit_failure:
    instance->force_exit = true;
    ipc_exit_loop(instance->loop_context);
}
