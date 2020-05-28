/**
 * \file supervisor/supervisor_create_canonizationservice.c
 *
 * \brief Create the canonization service.
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>

#include "supervisor_private.h"

/**
 * \brief Canonization service process structure.
 */
typedef struct canonization_process
{
    process_t hdr;
    const bootstrap_config_t* bconf;
    const agent_config_t* conf;
    int* data_socket;
    int* random_socket;
    int* log_socket;
    int control_socket;
    /* do not close the srv socket. */
    int control_srv_socket;
} canonization_process_t;

/* forward decls. */
static void supervisor_dispose_canonizationservice(void* disposable);
static int supervisor_start_canonizationservice(process_t* proc);

/**
 * \brief Create the canonization service as a process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the canonization service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              canonization service.  This configuration must
 *                              be valid for the lifetime of the service.
 * \param data_socket           The data socket descriptor.
 * \param random_socket         The random socket descriptor.
 * \param log_socket            The log socket descriptor.
 * \param control_socket        The control socket descriptor.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_canonizationservice(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, int* data_socket, int* random_socket,
    int* log_socket, int* control_socket)
{
    int retval;

    /* allocate memory for the canonization process. */
    canonization_process_t* canonization_proc =
        (canonization_process_t*)malloc(sizeof(canonization_process_t));
    if (NULL == canonization_proc)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set up canonization_proc structure. */
    memset(canonization_proc, 0, sizeof(canonization_process_t));
    canonization_proc->hdr.hdr.dispose =
        &supervisor_dispose_canonizationservice;
    canonization_proc->hdr.init_method = &supervisor_start_canonizationservice;
    canonization_proc->bconf = bconf;
    canonization_proc->conf = conf;
    canonization_proc->data_socket = data_socket;
    canonization_proc->random_socket = random_socket;
    canonization_proc->log_socket = log_socket;

    /* create the socketpair for the control socket. */
    retval =
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0, control_socket,
            &canonization_proc->control_socket);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_canonization_proc;
    }

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    canonization_proc->control_srv_socket = *control_socket;
    *svc = (process_t*)canonization_proc;
    goto done;

cleanup_canonization_proc:
    memset(canonization_proc, 0, sizeof(canonization_process_t));
    free(canonization_proc);

done:
    return retval;
}

/**
 * \brief Start the canonization service.
 *
 * \param proc      The canonization service to start.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 */
static int supervisor_start_canonizationservice(process_t* proc)
{
    canonization_process_t* canonization_proc = (canonization_process_t*)proc;
    int retval;
    uint32_t offset, status;

    /* attempt to create the canonization service. */
    TRY_OR_FAIL(
        start_canonization_proc(
            canonization_proc->bconf, canonization_proc->conf,
            canonization_proc->log_socket, canonization_proc->data_socket,
            canonization_proc->random_socket,
            &canonization_proc->control_socket,
            &canonization_proc->hdr.process_id,
            true),
        done);

    /* attempt to send config data to the canonization proc. */
    TRY_OR_FAIL(
        canonization_api_sendreq_configure(
            canonization_proc->control_srv_socket, canonization_proc->conf),
        terminate_proc);

    /* attempt to read the response from this init. */
    TRY_OR_FAIL(
        canonization_api_recvresp_configure(
            canonization_proc->control_srv_socket, &offset, &status),
        terminate_proc);

    /* verify that the configure operation completed successfully. */
    TRY_OR_FAIL(status, terminate_proc);

    /* attempt to start the canonization proc. */
    TRY_OR_FAIL(
        canonization_api_sendreq_start(
            canonization_proc->control_srv_socket),
        terminate_proc);

    /* attempt to read the response from this start. */
    TRY_OR_FAIL(
        canonization_api_recvresp_start(
            canonization_proc->control_srv_socket, &offset, &status),
        terminate_proc);

    /* verify that the start operation completed successfully. */
    TRY_OR_FAIL(status, terminate_proc);

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

terminate_proc:
    /* force the running status to true so we can terminate the process. */
    canonization_proc->hdr.running = true;
    process_stop((process_t*)canonization_proc);
    sleep(5);
    process_kill((process_t*)canonization_proc);

done:
    return retval;
}

/**
 * \brief Dispose of the canonization service by cleaning up.
 */
static void supervisor_dispose_canonizationservice(void* disposable)
{
    canonization_process_t* canonization_proc =
        (canonization_process_t*)disposable;

    /* clean up the log socket if valid. */
    if (*canonization_proc->log_socket > 0)
    {
        close(*canonization_proc->log_socket);
        *canonization_proc->log_socket = -1;
    }

    /* clean up the data socket if valid. */
    if (*canonization_proc->data_socket > 0)
    {
        close(*canonization_proc->data_socket);
        *canonization_proc->data_socket = -1;
    }

    /* clean up the random socket if valid. */
    if (*canonization_proc->random_socket > 0)
    {
        close(*canonization_proc->random_socket);
        *canonization_proc->random_socket = -1;
    }

    /* clean up the control socket if valid. */
    if (canonization_proc->control_socket > 0)
    {
        close(canonization_proc->control_socket);
        canonization_proc->control_socket = -1;
    }

    if (canonization_proc->hdr.running)
    {
        /* call the process stop method. */
        process_stop((process_t*)canonization_proc);

        sleep(5);

        /* kill the proc. */
        process_kill((process_t*)canonization_proc);
    }
}
