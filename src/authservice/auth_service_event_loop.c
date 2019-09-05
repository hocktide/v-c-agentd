/**
 * \file authservice/auth_service_event_loop.c
 *
 * \brief The event loop for the auth service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/authservice.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <errno.h>
#include <string.h>
#include <vpr/parameters.h>


#include "auth_service_private.h"

/* forward decls */
static void auth_service_ipc_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);
static void auth_service_ipc_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Event loop for the authentication service.  This is the entry point 
 * for the auth service.  It handles the details of reacting to events
 * sent over the auth service socket.
 *
 * \param authsock      The auth service socket.  The auth service listens for 
 *                      connections on this socket.
 * \param logsock       The logging service socket.  The auth service logs on 
 *                      this socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_AUTHSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *            attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if adding
 *            the auth service socket to the event loop failed.
 *          - AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if running
 *            the auth service event loop failed.
 */
int auth_service_event_loop(int authsock, int UNUSED(logsock))
{
    int retval = 0;

    auth_service_instance_t inst;

    MODEL_ASSERT(authsock >= 0);
    MODEL_ASSERT(logsock >= 0);

    /* initialize this instance. */
    retval = auth_service_instance_init(&inst, authsock);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* set the read callback for the auth socket. */
    ipc_set_readcb_noblock(
        &inst.auth, &auth_service_ipc_read, NULL);

    /* add the auth socket to the event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_add(inst.loop, &inst.auth))
    {
        retval = AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_ADD_FAILURE;
        goto cleanup_inst;
    }

    /* run the ipc event loop. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_run(inst.loop))
    {
        retval = AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_RUN_FAILURE;
        goto cleanup_inst;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_inst:
    dispose((disposable_t*)&inst);

done:
    return retval;
}


/**
 * \brief Handle read events on the auth socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this auth socket.
 */
static void auth_service_ipc_read(
    ipc_socket_context_t* ctx, int UNUSED(event_flags),
    void* user_context)
{
    ssize_t retval = 0;
    void* req;
    uint32_t size = 0;
    auth_service_instance_t* instance = (auth_service_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_READ);
    MODEL_ASSERT(NULL != instance);

    /* don't process data from this socket if we have been forced to exit. */
    if (instance->auth_service_force_exit)
        return;

    /* attempt to read a request. */
    retval = ipc_read_data_noblock(ctx, &req, &size);
    switch (retval)
    {
        /* on success, decode and dispatch. */
        case 0:
            /*if (0 != dataservice_decode_and_dispatch(instance, ctx, req, size))
            {
                instance->dataservice_force_exit = true;
                ipc_exit_loop(instance->loop_context);
            }*/

            /* clear and free the request data. */
            memset(req, 0, size);
            free(req);
            break;

        /* Wait for more data on the socket. */
        case AGENTD_ERROR_IPC_WOULD_BLOCK:
            break;

        /* any other error code indicates that we should no longer trust the
         * socket. */
        default:
            instance->auth_service_force_exit = true;
            ipc_exit_loop(instance->loop);
            break;
    }

    /* fire up the write callback if there is data to write. */
    if (ipc_socket_writebuffer_size(ctx) > 0)
    {
        ipc_set_writecb_noblock(ctx, &auth_service_ipc_write, instance->loop);
    }
}

/**
 * \brief Handle write events on the auth socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this auth socket.
 */
static void auth_service_ipc_write(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    auth_service_instance_t* instance = (auth_service_instance_t*)user_context;

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
    else
    {
        /* disable callback if there is no more data to write. */
        ipc_set_writecb_noblock(
            ctx, &auth_service_ipc_write, instance->loop);
    }

    /* success. */
    return;

exit_failure:
    instance->auth_service_force_exit = true;
    ipc_exit_loop(instance->loop);
}
