/**
 * \file supervisor/supervisor_create_random_service.c
 *
 * \brief Create the random service as a process that can be started.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/randomservice.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>

/**
 * \brief Random service process structure.
 */
typedef struct random_process
{
    process_t hdr;
    const bootstrap_config_t* bconf;
    const agent_config_t* conf;
    int* proto_random_socket;
    int* log_socket;
} random_process_t;

/* forward decls. */
static void supervisor_dispose_random_service(void* disposable);
static int supervisor_start_random_service(process_t* proc);

/**
 * \brief Create the random service as a process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the random service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              random service.  This configuration must be
 *                              valid for the lifetime of the service.
 * \param log_socket            The log socket descriptor.
 * \param proto_random_socket   The random socket descriptor for the proto svc.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_random_service(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, int* log_socket, int* proto_random_socket)
{
    int retval;

    /* allocate memory for the random process. */
    random_process_t* random_prc =
        (random_process_t*)malloc(sizeof(random_process_t));
    if (NULL == random_prc)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set up random_prc structure. */
    memset(random_prc, 0, sizeof(random_process_t));
    random_prc->hdr.hdr.dispose = &supervisor_dispose_random_service;
    random_prc->hdr.init_method = &supervisor_start_random_service;
    random_prc->bconf = bconf;
    random_prc->conf = conf;
    random_prc->proto_random_socket = proto_random_socket;
    random_prc->log_socket = log_socket;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    *svc = (process_t*)random_prc;
    goto done;

done:
    return retval;
}

/**
 * \brief Start the random service.
 *
 * \param proc      The random service to start.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 */
static int supervisor_start_random_service(process_t* proc)
{
    random_process_t* random_prc = (random_process_t*)proc;
    int retval;

    /* attempt to create the listener service. */
    TRY_OR_FAIL(
        randomservice_proc(
            random_prc->bconf, random_prc->conf,
            *random_prc->log_socket, random_prc->proto_random_socket,
            &random_prc->hdr.process_id, true),
        done);

    /* if successful, the child process owns the log socket. */
    *random_prc->log_socket = -1;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;

done:
    return retval;
}

/**
 * \brief Dispose of the data service by cleaning up.
 */
static void supervisor_dispose_random_service(void* disposable)
{
    random_process_t* random_prc = (random_process_t*)disposable;

    /* clean up the log socket if valid. */
    if (*random_prc->log_socket > 0)
    {
        close(*random_prc->log_socket);
        *random_prc->log_socket = -1;
    }

    /* clean up the proto random socket if valid. */
    if (*random_prc->proto_random_socket > 0)
    {
        close(*random_prc->proto_random_socket);
        *random_prc->proto_random_socket = -1;
    }

    /* call the process stop method. */
    process_stop((process_t*)random_prc);
}
