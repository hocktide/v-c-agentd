/**
 * \file supervisor/supervisor_create_listener_service.c
 *
 * \brief Create the listener service as a process that can be started.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/listenservice.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>

/**
 * \brief Listener process structure.
 */
typedef struct listener_process
{
    process_t hdr;
    const bootstrap_config_t* bconf;
    const agent_config_t* conf;
    int accept_lsocket;
    int* log_socket;
} listener_process_t;

/* forward decls. */
static void supervisor_dispose_listener_service(void* disposable);
static int supervisor_start_listener_service(process_t* proc);

/**
 * \brief Create the listener service as a process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the listener service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              listener service.  This configuration must be
 *                              valid for the lifetime of the service.
 * \param accept_socket         Pointer to the descriptor to receive the accept
 *                              socket.
 * \param log_socket            Pointer to the descriptor holding the log socket
 *                              for this instance.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_listener_service(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, int* accept_socket, int* log_socket)
{
    int retval;

    /* allocate memory for the listener process. */
    listener_process_t* listener_proc =
        (listener_process_t*)malloc(sizeof(listener_process_t));
    if (NULL == listener_proc)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set up listener_proc structure. */
    memset(listener_proc, 0, sizeof(listener_process_t));
    listener_proc->hdr.hdr.dispose = &supervisor_dispose_listener_service;
    listener_proc->hdr.init_method = &supervisor_start_listener_service;
    listener_proc->bconf = bconf;
    listener_proc->conf = conf;
    listener_proc->log_socket = log_socket;

    /* create a socket pair for sending the protocol service accepted
     * connections. */
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_DGRAM, 0,
            &listener_proc->accept_lsocket, accept_socket),
        cleanup_listener_proc);

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    *svc = (process_t*)listener_proc;
    goto done;

cleanup_listener_proc:
    dispose((disposable_t*)listener_proc);
    free(listener_proc);

done:
    return retval;
}

/**
 * \brief Start the listener service.
 *
 * \param proc      The listener service to start.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 */
static int supervisor_start_listener_service(process_t* proc)
{
    listener_process_t* listen_proc = (listener_process_t*)proc;
    int retval;

    /* attempt to create the listener service. */
    TRY_OR_FAIL(
        listenservice_proc(
            listen_proc->bconf, listen_proc->conf, listen_proc->accept_lsocket,
            *listen_proc->log_socket, &listen_proc->hdr.process_id, true),
        done);

    /* if successful, the child process owns the sockets. */
    *listen_proc->log_socket = -1;
    listen_proc->accept_lsocket = -1;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;

done:
    return retval;
}

/**
 * \brief Dispose of the data service by cleaning up.
 */
static void supervisor_dispose_listener_service(void* disposable)
{
    listener_process_t* listener = (listener_process_t*)disposable;

    /* clean up the accept socket if valid. */
    if (listener->accept_lsocket > 0)
    {
        close(listener->accept_lsocket);
        listener->accept_lsocket = -1;
    }

    /* clean up the log socket if valid. */
    if (*listener->log_socket > 0)
    {
        close(*listener->log_socket);
        *listener->log_socket = -1;
    }

    if (listener->hdr.running)
    {
        /* call the process stop method. */
        process_stop((process_t*)listener);

        sleep(5);

        /* kill the process. */
        process_kill((process_t*)listener);
    }
}
