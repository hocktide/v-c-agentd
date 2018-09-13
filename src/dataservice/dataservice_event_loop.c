/**
 * \file dataservice/dataservice_event_loop.c
 *
 * \brief The event loop for the data service.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/ipc.h>
#include <cbmc/model_assert.h>
#include <signal.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/* forward decls */
static void dataservice_ipc_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void dataservice_ipc_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Event loop for the data service.  This is the entry point for the data
 * service.  It handles the details of reacting to events sent over the data
 * service socket.
 *
 * \param datasock      The data service socket.  The data service listens for
 *                      requests on this socket and sends responses.
 * \param logsock       The logging service socket.  The data service logs data
 *                      on this socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - 0 on normal exit.
 *          - non-zero on abnormal exit.
 */
int dataservice_event_loop(int datasock, int UNUSED(logsock))
{
    int retval = 0;
    dataservice_instance_t* instance = NULL;
    ipc_socket_context_t data;
    ipc_event_loop_context_t loop;

    /* parameter sanity checking. */
    MODEL_ASSERT(datasock >= 0);
    MODEL_ASSERT(logsock >= 0);

    /* Create the dataservice instance. */
    instance = dataservice_instance_create();
    if (NULL == instance)
    {
        retval = 1;
        goto done;
    }

    /* set the data socket to non-blocking. */
    if (0 != ipc_make_noblock(datasock, &data, instance))
    {
        retval = 2;
        goto cleanup_instance;
    }

    /* initialize an IPC event loop instance. */
    if (0 != ipc_event_loop_init(&loop))
    {
        retval = 3;
        goto cleanup_datasock;
    }

    /* set a reference to the event loop in the instance. */
    instance->loop_context = &loop;

    /* set the read, write, and error callbacks for the data socket. */
    ipc_set_readcb_noblock(&data, &dataservice_ipc_read);
    ipc_set_writecb_noblock(&data, &dataservice_ipc_write);

    /* on these signals, leave the event loop and shut down gracefully. */
    ipc_exit_loop_on_signal(&loop, SIGHUP);
    ipc_exit_loop_on_signal(&loop, SIGTERM);
    ipc_exit_loop_on_signal(&loop, SIGQUIT);

    /* add the data socket to the event loop. */
    if (0 != ipc_event_loop_add(&loop, &data))
    {
        retval = 4;
        goto cleanup_loop;
    }

    /* run the ipc event loop. */
    if (0 != ipc_event_loop_run(&loop))
    {
        retval = 5;
        goto cleanup_loop;
    }

    /* success. */
    retval = 0;

cleanup_loop:
    dispose((disposable_t*)&loop);

cleanup_datasock:
    dispose((disposable_t*)&data);

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
 * \brief Handle read events on the data socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this data socket.
 */
static void dataservice_ipc_read(
    ipc_socket_context_t* ctx, int UNUSED(event_flags),
    void* user_context)
{
    ssize_t retval = 0;
    void* req;
    uint32_t size = 0;
    dataservice_instance_t* instance = (dataservice_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_READ);
    MODEL_ASSERT(NULL != instance);

    /* don't process data from this socket if we have been forced to exit. */
    if (instance->dataservice_force_exit)
        return;

    /* attempt to read a request. */
    retval = ipc_read_data_noblock(ctx, &req, &size);
    switch (retval)
    {
        /* on success, decode and dispatch. */
        case 0:
            if (0 != dataservice_decode_and_dispatch(instance, ctx, req, size))
            {
                instance->dataservice_force_exit = true;
                ipc_exit_loop(instance->loop_context);
            }

            /* clear and free the request data. */
            memset(req, 0, size);
            free(req);
            break;

        /* Wait for more data on the socket. */
        case IPC_ERROR_CODE_WOULD_BLOCK:
            return;

        /* any other error code indicates that we should no longer trust the
         * socket. */
        default:
            instance->dataservice_force_exit = true;
            ipc_exit_loop(instance->loop_context);
            break;
    }

    /* kick the write callback to ensure that it runs at least once. */
    dataservice_ipc_write(ctx, IPC_SOCKET_EVENT_WRITE, user_context);
}

/**
 * \brief Handle write events on the data socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this data socket.
 */
static void dataservice_ipc_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    dataservice_instance_t* instance = (dataservice_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_READ);
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

    /* success. */
    return;

exit_failure:
    instance->dataservice_force_exit = true;
    ipc_exit_loop(instance->loop_context);
}
