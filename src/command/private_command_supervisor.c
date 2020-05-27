/**
 * \file command/private_command_supervisor.c
 *
 * \brief Create, spawn, and introduce all services.
 *
 * \copyright 2018-2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/control.h>
#include <agentd/config.h>
#include <agentd/ipc.h>
#include <agentd/process.h>
#include <agentd/status_codes.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <vpr/parameters.h>

/* forward decls. */
static int supervisor_run();

/**
 * \brief Run the the supervisor.
 */
void private_command_supervisor(bootstrap_config_t* bconf)
{
    /* install the signal handlers. */
    if (AGENTD_STATUS_SUCCESS != supervisor_sighandler_install())
    {
        perror("supervisor_sighandler_install");
        return;
    }

    /* we are in the running state. */
    keep_running = true;

    /* TODO - set the process name. */

    while (keep_running)
    {
        /* if supervisor_run fails, exit. */
        if (AGENTD_STATUS_SUCCESS != supervisor_run(bconf))
        {
            keep_running = false;
        }
    }

    /* uninstall the signal handlers on exit. */
    supervisor_sighandler_uninstall();
}

/**
 * \brief Helper macro for starting a process.
 */
#define START_PROCESS(svc, label) \
    do \
    { \
        retval = process_start(svc); \
        if (AGENTD_STATUS_SUCCESS != retval) \
        { \
            goto label; \
        } \
    } while (0)

/**
 * \brief Helper macro for closing a socket if valid.
 */
#define CLOSE_IF_VALID(sock) \
    if (sock >= 0) \
    { \
        close(sock); \
    }

/**
 * \brief Helper macro to clean up process.
 */
#define CLEANUP_PROCESS(svc) \
    dispose((disposable_t*)svc); \
    free(svc)

/**
 * \brief Run the supervisor.
 *
 * \param bconf         The bootstrap config for the supervisor.
 *
 * This function attempts to bootstrap all child services and then waits until
 * an appropriate signal is detected prior to exiting.
 *
 * The bootstrap process first reads the configuration file and then uses this
 * configuration file to start each child service.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - A non-zero status code on failure.
 */
static int supervisor_run(const bootstrap_config_t* bconf)
{
    int retval = AGENTD_STATUS_SUCCESS;
    agent_config_t conf;
    process_t* random_service;
    process_t* random_for_canonizationservice;
    process_t* listener_service;
    process_t* data_for_auth_protocol_service;
    process_t* data_for_canonizationservice;
    process_t* protocol_service;
    process_t* canonizationservice;

    int random_svc_log_sock = -1;
    int random_svc_log_dummy_sock = -1;
    int random_svc_for_canonization_log_sock = -1;
    int random_svc_for_canonization_log_dummy_sock = -1;
    int listen_svc_log_sock = -1;
    int listen_svc_log_dummy_sock = -1;
    int unauth_protocol_svc_log_sock = -1;
    int unauth_protocol_svc_log_dummy_sock = -1;
    int data_for_auth_protocol_svc_log_sock = -1;
    int data_for_auth_protocol_svc_log_dummy_sock = -1;
    int data_for_canonization_svc_log_sock = -1;
    int data_for_canonization_svc_log_dummy_sock = -1;
    int unauth_protocol_svc_random_sock = -1;
    int unauth_protocol_svc_accept_sock = -1;
    int auth_protocol_svc_data_sock = -1;
    int canonization_svc_data_sock = -1;
    int canonization_svc_random_sock = -1;
    int canonization_svc_log_sock = -1;
    int canonization_svc_log_dummy_sock = -1;
    int canonization_svc_control_sock = -1;

#if AUTHSERVICE
    process_t* auth_service;

    int auth_svc_sock = -1;
    int auth_svc_log_sock = -1;
    int auth_svc_log_dummy_sock = -1;
#endif /*AUTHSERVICE*/

    /* read config. */
    TRY_OR_FAIL(config_read_proc(bconf, &conf), done);

    /* TODO - replace with log service. */
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &random_svc_log_sock, &random_svc_log_dummy_sock),
        cleanup_config);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &random_svc_for_canonization_log_sock,
            &random_svc_for_canonization_log_dummy_sock),
        cleanup_config);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &listen_svc_log_sock, &listen_svc_log_dummy_sock),
        cleanup_config);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &unauth_protocol_svc_log_sock,
            &unauth_protocol_svc_log_dummy_sock),
        cleanup_config);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &data_for_auth_protocol_svc_log_sock,
            &data_for_auth_protocol_svc_log_dummy_sock),
        cleanup_config);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &data_for_canonization_svc_log_sock,
            &data_for_canonization_svc_log_dummy_sock),
        cleanup_config);
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &canonization_svc_log_sock,
            &canonization_svc_log_dummy_sock),
        cleanup_config);
#if AUTHSERVICE
    TRY_OR_FAIL(
        ipc_socketpair(
            AF_UNIX, SOCK_STREAM, 0,
            &auth_svc_log_sock,
            &auth_svc_log_dummy_sock),
        cleanup_config);
#endif /*AUTHSERVICE*/

    /* create random service. */
    TRY_OR_FAIL(
        supervisor_create_random_service(
            &random_service, bconf, &conf, &random_svc_log_sock,
            &unauth_protocol_svc_random_sock),
        cleanup_config);

    /* create random service for canonization service. */
    TRY_OR_FAIL(
        supervisor_create_random_service(
            &random_for_canonizationservice, bconf, &conf,
            &random_svc_for_canonization_log_sock,
            &canonization_svc_random_sock),
        cleanup_random_service);

    /* create listener service. */
    TRY_OR_FAIL(
        supervisor_create_listener_service(
            &listener_service, bconf, &conf, &unauth_protocol_svc_accept_sock,
            &listen_svc_log_sock),
        cleanup_random_for_canonizationservice);

    /* create data service for protocol service. */
    TRY_OR_FAIL(
        supervisor_create_data_service_for_auth_protocol_service(
            &data_for_auth_protocol_service, bconf, &conf,
            &auth_protocol_svc_data_sock, &data_for_auth_protocol_svc_log_sock),
        cleanup_listener_service);

    /* create protocol service. */
    TRY_OR_FAIL(
        supervisor_create_protocol_service(
            &protocol_service, bconf, &conf, &unauth_protocol_svc_random_sock,
            &unauth_protocol_svc_accept_sock, &auth_protocol_svc_data_sock,
            &unauth_protocol_svc_log_sock),
        cleanup_data_for_auth_protocol_service);

#if AUTHSERVICE
    /* create auth service */
    TRY_OR_FAIL(
        supervisor_create_auth_service(
            &auth_service, bconf, &conf, &auth_svc_sock,
            &auth_svc_log_sock),
        cleanup_protocol_service);

    /* create data service for canonization service. */
    TRY_OR_FAIL(
        supervisor_create_data_service_for_canonizationservice(
            &data_for_canonizationservice, bconf, &conf,
            &canonization_svc_data_sock, &data_for_canonization_svc_log_sock),
        cleanup_auth_service);
#else
    /* create data service for canonization service. */
    TRY_OR_FAIL(
        supervisor_create_data_service_for_canonizationservice(
            &data_for_canonizationservice, bconf, &conf,
            &canonization_svc_data_sock, &data_for_canonization_svc_log_sock),
        cleanup_protocol_service);
#endif /*AUTHSERVICE*/

    /* create canonization service. */
    TRY_OR_FAIL(
        supervisor_create_canonizationservice(
            &canonizationservice, bconf, &conf, &canonization_svc_data_sock,
            &canonization_svc_random_sock, &canonization_svc_log_sock,
            &canonization_svc_control_sock),
        cleanup_data_service_for_canonizationservice);

    /* if we've made it this far, attempt to start each service. */
    START_PROCESS(random_service, cleanup_canonizationservice);
    START_PROCESS(random_for_canonizationservice, cleanup_canonizationservice);
    START_PROCESS(data_for_canonizationservice, cleanup_canonizationservice);
    START_PROCESS(data_for_auth_protocol_service, quiesce_data_processes);
    START_PROCESS(listener_service, quiesce_data_processes);

#if AUTHSERVICE
    START_PROCESS(auth_service, quiesce_data_processes);
#endif /*AUTHSERVICE*/
    START_PROCESS(protocol_service, quiesce_data_processes);
    START_PROCESS(canonizationservice, quiesce_data_processes);

    /* wait until we get a signal, and then restart / terminate. */
    supervisor_sighandler_wait();

    /* wait before shutting everything down. */
    sleep(5);

    /* quiesce the higher-level processes first. */
#if AUTHSERVICE
    process_stop(auth_service);
#endif /*AUTHSERVICE*/
    process_stop(listener_service);
    process_stop(protocol_service);
    process_stop(canonizationservice);

quiesce_data_processes:
    process_stop(data_for_canonizationservice);
    process_stop(data_for_auth_protocol_service);

cleanup_canonizationservice:
    CLEANUP_PROCESS(canonizationservice);

cleanup_data_service_for_canonizationservice:
    CLEANUP_PROCESS(data_for_canonizationservice);

#if AUTHSERVICE
cleanup_auth_service:
    CLEANUP_PROCESS(auth_service);
#endif /*AUTHSERVICE*/

cleanup_protocol_service:
    CLEANUP_PROCESS(protocol_service);

cleanup_data_for_auth_protocol_service:
    CLEANUP_PROCESS(data_for_auth_protocol_service);

cleanup_listener_service:
    CLEANUP_PROCESS(listener_service);

cleanup_random_for_canonizationservice:
    CLEANUP_PROCESS(random_for_canonizationservice);

cleanup_random_service:
    CLEANUP_PROCESS(random_service);

cleanup_config:
    dispose((disposable_t*)&conf);

done:
    CLOSE_IF_VALID(random_svc_log_sock);
    CLOSE_IF_VALID(random_svc_log_dummy_sock);
    CLOSE_IF_VALID(random_svc_for_canonization_log_sock);
    CLOSE_IF_VALID(random_svc_for_canonization_log_dummy_sock);
    CLOSE_IF_VALID(listen_svc_log_sock);
    CLOSE_IF_VALID(listen_svc_log_dummy_sock);
    CLOSE_IF_VALID(unauth_protocol_svc_log_sock);
    CLOSE_IF_VALID(unauth_protocol_svc_log_dummy_sock);
    CLOSE_IF_VALID(data_for_auth_protocol_svc_log_sock);
    CLOSE_IF_VALID(data_for_auth_protocol_svc_log_dummy_sock);
    CLOSE_IF_VALID(data_for_canonization_svc_log_sock);
    CLOSE_IF_VALID(data_for_canonization_svc_log_dummy_sock);
    CLOSE_IF_VALID(unauth_protocol_svc_random_sock);
    CLOSE_IF_VALID(unauth_protocol_svc_accept_sock);
    CLOSE_IF_VALID(auth_protocol_svc_data_sock);
    CLOSE_IF_VALID(canonization_svc_data_sock);
    CLOSE_IF_VALID(canonization_svc_random_sock);
    CLOSE_IF_VALID(canonization_svc_log_sock);
    CLOSE_IF_VALID(canonization_svc_log_dummy_sock);
    CLOSE_IF_VALID(canonization_svc_control_sock);

#if AUTHSERVICE
    CLOSE_IF_VALID(auth_svc_log_sock);
    CLOSE_IF_VALID(auth_svc_log_dummy_sock);
    CLOSE_IF_VALID(auth_svc_sock);
#endif /*AUTHSERVICE*/

    return retval;
}
