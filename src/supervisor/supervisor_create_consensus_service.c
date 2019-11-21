/**
 * \file supervisor/supervisor_create_consensus_service.c
 *
 * \brief Create the consensus service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/consensusservice.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>

#include "supervisor_private.h"

/**
 * \brief Consensus service process structure.
 */
typedef struct consensus_process
{
    process_t hdr;
    const bootstrap_config_t* bconf;
    const agent_config_t* conf;
    int* data_socket;
    int* log_socket;
} consensus_process_t;

/* forward decls. */
static void supervisor_dispose_consensus_service(void* disposable);
static int supervisor_start_consensus_service(process_t* proc);

/**
 * \brief Create the consensus service as a process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the consensus service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              consensus service.  This configuration must be
 *                              valid for the lifetime of the service.
 * \param data_socket           The data socket descriptor.
 * \param log_socket            The log socket descriptor.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_consensus_service(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, int* data_socket, int* log_socket)
{
    int retval;

    /* allocate memory for the consensus process. */
    consensus_process_t* consensus_proc =
        (consensus_process_t*)malloc(sizeof(consensus_process_t));
    if (NULL == consensus_proc)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set up consensus_proc structure. */
    memset(consensus_proc, 0, sizeof(consensus_process_t));
    consensus_proc->hdr.hdr.dispose = &supervisor_dispose_consensus_service;
    consensus_proc->hdr.init_method = &supervisor_start_consensus_service;
    consensus_proc->bconf = bconf;
    consensus_proc->conf = conf;
    consensus_proc->data_socket = data_socket;
    consensus_proc->log_socket = log_socket;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    *svc = (process_t*)consensus_proc;
    goto done;

done:
    return retval;
}

/**
 * \brief Start the consensus service.
 *
 * \param proc      The consensus service to start.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 */
static int supervisor_start_consensus_service(process_t* proc)
{
    consensus_process_t* consensus_proc = (consensus_process_t*)proc;
    int retval;

    /* attempt to create the consensus service. */
    TRY_OR_FAIL(
        start_consensus_proc(
            consensus_proc->bconf, consensus_proc->conf,
            *consensus_proc->log_socket, *consensus_proc->data_socket,
            &consensus_proc->hdr.process_id, true),
        done);

    /* if successful, the child process owns the sockets. */
    *consensus_proc->log_socket = -1;
    *consensus_proc->data_socket = -1;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;

done:
    return retval;
}

/**
 * \brief Dispose of the consensus service by cleaning up.
 */
static void supervisor_dispose_consensus_service(void* disposable)
{
    consensus_process_t* consensus_proc = (consensus_process_t*)disposable;

    /* clean up the log socket if valid. */
    if (*consensus_proc->log_socket > 0)
    {
        close(*consensus_proc->log_socket);
        *consensus_proc->log_socket = -1;
    }

    /* clean up the data socket if valid. */
    if (*consensus_proc->data_socket > 0)
    {
        close(*consensus_proc->data_socket);
        *consensus_proc->data_socket = -1;
    }

    /* call the process stop method. */
    process_stop((process_t*)consensus_proc);
}
