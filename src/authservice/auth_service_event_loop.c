/**
 * \file authservice/auth_service_event_loop.c
 *
 * \brief The event loop for the auth service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/authservice/private/authservice.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <errno.h>
#include <string.h>
#include <vpr/parameters.h>

#include "auth_service_private.h"

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
