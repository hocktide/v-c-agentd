/**
 * \file supervisor/supervisor_create_auth_service.c
 *
 * \brief Create the auth service as a process that can be started.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/authservice.h>
#include <agentd/control.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>

/**
 * \brief Auth service process structure.
 */
typedef struct auth_process
{
    process_t hdr;
    const bootstrap_config_t* bconf;
    const agent_config_t* conf;
    int* auth_socket;
    int* log_socket;
} auth_process_t;

/* forward decls. */
static void supervisor_dispose_auth_service(void* disposable);
static int supervisor_start_auth_service(process_t* proc);

/**
 * \brief Create the auth service as a process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the auth service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              auth service.  This configuration must be
 *                              valid for the lifetime of the service.
 * \param auth_socket           The auth socket descriptor.
 * \param log_socket            The log socket descriptor.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_auth_service(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, int* auth_socket, int* log_socket)
{
    int retval;

    /* allocate memory for the auth process. */
    auth_process_t* auth_proc =
        (auth_process_t*)malloc(sizeof(auth_process_t));
    if (NULL == auth_proc)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set up auth_proc structure. */
    memset(auth_proc, 0, sizeof(auth_process_t));
    auth_proc->hdr.hdr.dispose = &supervisor_dispose_auth_service;
    auth_proc->hdr.init_method = &supervisor_start_auth_service;
    auth_proc->bconf = bconf;
    auth_proc->conf = conf;
    auth_proc->auth_socket = auth_socket;
    auth_proc->log_socket = log_socket;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    *svc = (process_t*)auth_proc;
    goto done;

done:
    return retval;
}

/**
 * \brief Start the auth service.
 *
 * \param proc      The auth service to start.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 */
static int supervisor_start_auth_service(process_t* proc)
{
    auth_process_t* auth_proc = (auth_process_t*)proc;
    int retval;

    /* attempt to create the auth service. */
    TRY_OR_FAIL(
        auth_service_proc(
            auth_proc->bconf, auth_proc->conf,
            *auth_proc->log_socket, auth_proc->auth_socket,
            &auth_proc->hdr.process_id, true),
        done);

    /* if successful, the child process owns the sockets. */
    *auth_proc->auth_socket = -1;
    *auth_proc->log_socket = -1;

    /* TODO: initialize auth service with agent ID, and public and private keys */


    /* success */
    retval = AGENTD_STATUS_SUCCESS;

done:
    return retval;
}

/**
 * \brief Dispose of the auth service by cleaning up.
 */
static void supervisor_dispose_auth_service(void* disposable)
{
    auth_process_t* auth_proc = (auth_process_t*)disposable;

    /* clean up the accept socket if valid. */
    if (*auth_proc->auth_socket > 0)
    {
        close(*auth_proc->auth_socket);
        *auth_proc->auth_socket = -1;
    }

    /* clean up the log socket if valid. */
    if (*auth_proc->log_socket > 0)
    {
        close(*auth_proc->log_socket);
        *auth_proc->log_socket = -1;
    }

    /* call the process stop method. */
    process_stop((process_t*)auth_proc);
}
