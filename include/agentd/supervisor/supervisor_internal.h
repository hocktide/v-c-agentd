/**
 * \file agentd/supervisor/supervisor_internal.h
 *
 * \brief Internal supervisor functions for setting up services.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_SUPERVISOR_SUPERVISOR_INTERNAL_HEADER_GUARD
#define AGENTD_SUPERVISOR_SUPERVISOR_INTERNAL_HEADER_GUARD

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include <agentd/config.h>
#include <agentd/process.h>

/**
 * \brief Create the random service as a process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the protocol service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              protocol service.  This configuration must be
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
    const agent_config_t* conf, int* log_socket, int* proto_random_socket);

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
    const agent_config_t* conf, int* accept_socket, int* log_socket);

/**
 * \brief Create a data service instance for the authenticated protocol as a
 * process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the data service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              data service.  This configuration must be
 *                              valid for the lifetime of the service.
 * \param data_socket           Pointer to the descriptor to receive the data
 *                              socket.
 * \param log_socket            Pointer to the descriptor holding the log socket
 *                              for this instance.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_data_service_for_auth_protocol_service(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, int* data_socket, int* log_socket);

/**
 * \brief Create a data service instance for the consensus service as a
 * process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the data service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              data service.  This configuration must be
 *                              valid for the lifetime of the service.
 * \param data_socket           Pointer to the descriptor to receive the data
 *                              socket.
 * \param log_socket            Pointer to the descriptor holding the log socket
 *                              for this instance.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_data_service_for_consensus_service(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, int* data_socket, int* log_socket);

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
    int* data_socket, int* log_socket);

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
    const agent_config_t* conf, int* auth_socket, int* log_socket);

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
 * \param control_socket        The control socket descriptor, to be updated to
 *                              the control socket for this process on
 *                              successful completion of this method.
 * 
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_consensus_service(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, int* data_socket, int* log_socket,
    int* control_socket);

/**
 * \brief Install the signal handler for the supervisor.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_sighandler_install();

/**
 * \brief Uninstall signal handlers.
 */
void supervisor_sighandler_uninstall();

/**
 * \brief Wait until a signal occurs.
 */
void supervisor_sighandler_wait();

/**
 * \brief Signal handler for the supervisor process.
 */
void supervisor_signal_handler(int signal);

/* flag to indicate whether we should continue running. */
extern bool keep_running;

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*AGENTD_SUPERVISOR_SUPERVISOR_INTERNAL_HEADER_GUARD*/
