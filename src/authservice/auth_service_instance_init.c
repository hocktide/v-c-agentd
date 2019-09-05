/**
 * \file authservice/auth_service_instance_init.c
 *
 * \brief Initialize the authentication service instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/parameters.h>

#include "auth_service_private.h"

/* forward decls */
static void auth_service_instance_dispose(void* disposable);


/**
 * \brief Create the auth service instance.
 *
 * \param inst          The service instance to initialize.
 * \param auth          The auth socket to use for this instance.
 *
 * \returns a status code indicating success or failure.
 */
int auth_service_instance_init(auth_service_instance_t* inst, int auth)
{
    int retval = AGENTD_STATUS_SUCCESS;
    ipc_event_loop_context_t loop;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(auth >= 0);
    MODEL_ASSERT(max_socks > 0);

    /* Set up the instance basics. */
    memset(inst, 0, sizeof(auth_service_instance_t));
    inst->hdr.dispose = &auth_service_instance_dispose;

    /* create the allocator for this instance. */
    malloc_allocator_options_init(&inst->alloc_opts);

    /* create the crypto suite for this instance. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_suite_options_init(
            &inst->suite, &inst->alloc_opts, VCCRYPT_SUITE_VELO_V1))
    {
        retval = AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_allocator;
    }

    /* set the auth socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS != ipc_make_noblock(auth, &inst->auth, inst))
    {
        retval = AGENTD_ERROR_AUTHSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_suite;
    }

    /* initialize the IPC event loop instance. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_init(&loop))
    {
        retval = AGENTD_ERROR_AUTHSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_auth;
    }

    /* set a reference to the event loop in the instance. */
    inst->loop = &loop;

    /* on these signals, leave the event loop and shut down gracefully. */
    ipc_exit_loop_on_signal(&loop, SIGHUP);
    ipc_exit_loop_on_signal(&loop, SIGTERM);
    ipc_exit_loop_on_signal(&loop, SIGQUIT);


    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;


    /* clean up resources if an error occurred */
    dispose((disposable_t*)&loop);

cleanup_auth:
    dispose((disposable_t*)&inst->auth);

cleanup_suite:
    dispose((disposable_t*)&inst->suite);

cleanup_allocator:
    dispose((disposable_t*)&inst->alloc_opts);

done:
    return retval;
}

/**
 * \brief Dispose of an auth service instance.
 */
static void auth_service_instance_dispose(void* disposable)
{
    auth_service_instance_t* inst = (auth_service_instance_t*)disposable;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != inst);

    /* dispose of the auth socket. */
    dispose((disposable_t*)&inst->auth);

    /* dispose of the loop. */
    dispose((disposable_t*)&inst->loop);

    /* dispose of the crypto suite. */
    dispose((disposable_t*)&inst->suite);

    /* dispose of the allocator. */
    dispose((disposable_t*)&inst->alloc_opts);

    /* clear this instance. */
    memset(inst, 0, sizeof(auth_service_instance_t));
}
