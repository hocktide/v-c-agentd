/**
 * \file supervisor/supervisor_create_protocol_service.c
 *
 * \brief Create the protocol service as a process that can be started.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/protocolservice.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>

/**
 * \brief Protocol service process structure.
 */
typedef struct protocol_process
{
    process_t hdr;
    const bootstrap_config_t* bconf;
    const agent_config_t* conf;
    int* random_socket;
    int* accept_socket;
    int* data_socket;
    int* log_socket;
} protocol_process_t;

/* forward decls. */
static void supervisor_dispose_protocol_service(void* disposable);
static int supervisor_start_protocol_service(process_t* proc);

/**
 * \brief Create the protocol service as a process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the protocol service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              protocol service.  This configuration must be
 *                              valid for the lifetime of the service.
 * \param random_socket         The random socket descriptor.
 * \param accept_socket         The accept socket descriptor.
 * \param data_socket           The data socket descriptor.
 * \param log_socket            The log socket descriptor.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_protocol_service(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, int* random_socket, int* accept_socket,
    int* data_socket, int* log_socket)
{
    int retval;

    /* allocate memory for the protocol process. */
    protocol_process_t* protocol_proc =
        (protocol_process_t*)malloc(sizeof(protocol_process_t));
    if (NULL == protocol_proc)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set up protocol_proc structure. */
    memset(protocol_proc, 0, sizeof(protocol_process_t));
    protocol_proc->hdr.hdr.dispose = &supervisor_dispose_protocol_service;
    protocol_proc->hdr.init_method = &supervisor_start_protocol_service;
    protocol_proc->bconf = bconf;
    protocol_proc->conf = conf;
    protocol_proc->random_socket = random_socket;
    protocol_proc->accept_socket = accept_socket;
    protocol_proc->data_socket = data_socket;
    protocol_proc->log_socket = log_socket;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    *svc = (process_t*)protocol_proc;
    goto done;

done:
    return retval;
}

/**
 * \brief Start the protocol service.
 *
 * \param proc      The protocol service to start.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 */
static int supervisor_start_protocol_service(process_t* proc)
{
    protocol_process_t* protocol_proc = (protocol_process_t*)proc;
    int retval;

    /* attempt to create the listener service. */
    TRY_OR_FAIL(
        unauthorized_protocol_proc(
            protocol_proc->bconf, protocol_proc->conf,
            *protocol_proc->random_socket, *protocol_proc->log_socket,
            *protocol_proc->accept_socket, *protocol_proc->data_socket,
            &protocol_proc->hdr.process_id, true),
        done);

    /* if successful, the child process owns the sockets. */
    *protocol_proc->random_socket = -1;
    *protocol_proc->log_socket = -1;
    *protocol_proc->accept_socket = -1;
    *protocol_proc->data_socket = -1;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;

done:
    return retval;
}

/**
 * \brief Dispose of the data service by cleaning up.
 */
static void supervisor_dispose_protocol_service(void* disposable)
{
    protocol_process_t* protocol_proc = (protocol_process_t*)disposable;

    /* clean up the random socket if valid. */
    if (*protocol_proc->random_socket > 0)
    {
        close(*protocol_proc->random_socket);
        *protocol_proc->random_socket = -1;
    }

    /* clean up the accept socket if valid. */
    if (*protocol_proc->accept_socket > 0)
    {
        close(*protocol_proc->accept_socket);
        *protocol_proc->accept_socket = -1;
    }

    /* clean up the log socket if valid. */
    if (*protocol_proc->log_socket > 0)
    {
        close(*protocol_proc->log_socket);
        *protocol_proc->log_socket = -1;
    }

    /* clean up the data socket if valid. */
    if (*protocol_proc->data_socket > 0)
    {
        close(*protocol_proc->data_socket);
        *protocol_proc->data_socket = -1;
    }

    if (protocol_proc->hdr.running)
    {
        /* call the process stop method. */
        process_stop((process_t*)protocol_proc);

        sleep(5);

        /* kill the process. */
        process_kill((process_t*)protocol_proc);
    }
}
