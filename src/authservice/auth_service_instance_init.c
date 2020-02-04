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

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(auth >= 0);
    MODEL_ASSERT(max_socks > 0);

    /* set up the instance basics. */
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
        goto dispose_allocator;
    }

    /* initialize the public key crypto buffer */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_suite_buffer_init_for_auth_key_agreement_public_key(
            &inst->suite, &inst->agent_pubkey))
    {
        retval = AGENTD_ERROR_AUTHSERVICE_PUBKEY_BUFFER_INIT_FAILURE;
        goto dispose_suite;
    }

    /* create agent privkey buffer. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_suite_buffer_init_for_auth_key_agreement_private_key(
            &inst->suite, &inst->agent_privkey))
    {
        retval = AGENTD_ERROR_AUTHSERVICE_PRIVKEY_BUFFER_INIT_FAILURE;
        goto dispose_pubkey;
    }

    /* set the auth socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS != ipc_make_noblock(auth, &inst->auth, inst))
    {
        retval = AGENTD_ERROR_AUTHSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto dispose_privkey;
    }

    /* initialize the IPC event loop instance. */
    inst->loop = (ipc_event_loop_context_t*)allocate(
        &inst->alloc_opts, sizeof(ipc_event_loop_context_t));
    if (NULL == inst->loop)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto dispose_auth;
    }

    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_init(inst->loop))
    {
        retval = AGENTD_ERROR_AUTHSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto release_loop;
    }

    /* on these signals, leave the event loop and shut down gracefully. */
    ipc_exit_loop_on_signal(inst->loop, SIGHUP);
    ipc_exit_loop_on_signal(inst->loop, SIGTERM);
    ipc_exit_loop_on_signal(inst->loop, SIGQUIT);


    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;


    /* clean up resources if an error occurred */
release_loop:
    release(&inst->alloc_opts, inst->loop);

dispose_auth:
    dispose((disposable_t*)&inst->auth);

dispose_privkey:
    dispose((disposable_t*)&inst->agent_privkey);

dispose_pubkey:
    dispose((disposable_t*)&inst->agent_pubkey);

dispose_suite:
    dispose((disposable_t*)&inst->suite);

dispose_allocator:
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

    /* dispose of the loop. */
    dispose((disposable_t*)inst->loop);
    release(&inst->alloc_opts, inst->loop);

    /* dispose of the auth socket. */
    dispose((disposable_t*)&inst->auth);

    /* dispose of the private key crypto buffer */
    dispose((disposable_t*)&inst->agent_privkey);

    /* dispose of the public key crypto buffer */
    dispose((disposable_t*)&inst->agent_pubkey);

    /* dispose of the crypto suite. */
    dispose((disposable_t*)&inst->suite);

    /* dispose of the allocator. */
    dispose((disposable_t*)&inst->alloc_opts);

    /* clear this instance. */
    memset(inst, 0, sizeof(auth_service_instance_t));
}
