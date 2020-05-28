/**
 * \file agentd/authservice.h
 *
 * \brief Service level API for the auth service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_AUTHSERVICE_HEADER_GUARD
#define AGENTD_AUTHSERVICE_HEADER_GUARD

#include <agentd/bootstrap_config.h>
#include <agentd/config.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus


/**
 * \brief Auth service API methods.
 */
enum authservice_api_method_enum
{
    /**
     * \brief Lower bound of API methods.  Must be the first value in this
     * enumeration.
     */
    AUTHSERVICE_API_METHOD_LOWER_BOUND,

    /**
     * \brief Initialize the auth service.  Must proceed directly after lower 
     * bound.
     */
    AUTHSERVICE_API_METHOD_INITIALIZE =
        AUTHSERVICE_API_METHOD_LOWER_BOUND,

    /**
     * \brief The number of methods in this API.
     *
     * Must be immediately after the last enumerated bit value.
     */
    AUTHSERVICE_API_METHOD_UPPER_BOUND
};

/**
 * \brief Event loop for the authentication service.  This is the entry point 
 * for the auth service.  It handles the details of reacting to events
 * sent over the auth service socket.
 *
 * \param authsock      The auth service socket.  The auth service listens for 
 *                      connections on this socket.
 * \param logsock       The logging service socket.  The auth service logs on 
 *                      this socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_AUTHSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *            attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if adding
 *            the auth service socket to the event loop failed.
 *          - AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if running
 *            the auth service event loop failed.
 */
int auth_service_event_loop(int authsock, int logsock);

/**
 * \brief Spawn an auth service process using the provided config structure and
 * logger socket.
 *
 * On success, this method sets the file descriptor pointer to the file
 * descriptor for the auth service socket.  This can be used by the caller to
 * send requests to the auth service and to receive responses from this service.
 * Also, the pointer to the pid for this process is set.  This can be used to
 * signal and wait when this process should be terminated.
 *
 * \param bconf         The bootstrap configuration for this service.
 * \param conf          The configuration for this service.
 * \param logsock       Pointer to the socket used to communicate with the
 *                      logger.
 * \param authsock      Pointer to the auth service socket, to be updated on
 *                      successful completion of this function.
 * \param authpid       Pointer to the auth service pid, to be updated on the
 *                      successful completion of this function.
 * \param runsecure     Set to false if we are not being run in secure mode.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_AUTHSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED if spawning
 *        this process failed because the user is not root and runsecure is
 *        true.
 *      - AGENTD_ERROR_AUTHSERVICE_IPC_SOCKETPAIR_FAILURE if creating a
 *        socketpair for the dataservice process failed.
 *      - AGENTD_ERROR_AUTHSERVICE_FORK_FAILURE if forking the private process
 *        failed.
 *      - AGENTD_ERROR_AUTHSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE if there was
 *        a failure looking up the configured user and group for the dataservice
 *        process.
 *      - AGENTD_ERROR_AUTHSERVICE_PRIVSEP_CHROOT_FAILURE if chrooting failed.
 *      - AGENTD_ERROR_AUTHSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE if dropping
 *        privileges failed.
 *      - AGENTD_ERROR_AUTHSERVICE_PRIVSEP_SETFDS_FAILURE if setting file
 *        descriptors failed.
 *      - AGENTD_ERROR_AUTHSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE if executing the
 *        private command failed.
 *      - AGENTD_ERROR_AUTHSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS if the
 *        process survived execution (weird!).      
 */
int auth_service_proc(
    const bootstrap_config_t* bconf, const agent_config_t* conf, int* logsock,
    int* authsock, pid_t* authpid, bool runsecure);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_AUTHSERVICE_HEADER_GUARD*/
