/**
 * \file agentd/canonizationservice.h
 *
 * \brief Service level API for the canonization service.
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_CANONIZATIONSERVICE_HEADER_GUARD
#define AGENTD_CANONIZATIONSERVICE_HEADER_GUARD

#include <agentd/bootstrap_config.h>
#include <agentd/config.h>
#include <agentd/protocolservice/api.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Event loop for the canonization service.  This is the entry point for
 * the canonization service.
 *
 * \param datasock      The data service socket.  The canonization service
 *                      communicates with the dataservice using this socket.
 * \param randomsock    The random service socket.  The canonization service
 *                      communicates with the random service using this socket.
 * \param logsock       The logging service socket.  The canonization service
 *                      logs on this socket.
 * \param controlsock   The socket used to control the canonization service.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *            attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if
 *            adding the protocol service socket to the event loop failed.
 *          - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if
 *            running the protocol service event loop failed.
 */
int canonizationservice_event_loop(
    int datasock, int randomsock, int logsock, int controlsock);

/**
 * \brief Spawn a canonization service process using the provided config
 * structure and logger socket.
 *
 * On success, the pointer to the pid for this process is set.  This can be used
 * to signal and wait when this process should be terminated.
 *
 * \param bconf             The bootstrap configuration for this service.
 * \param conf              The configuration for this service.
 * \param logsock           Pointer to the socket used to communicate with the
 *                          logger.
 * \param datasock          Pointer to the socket used to communicate with the
 *                          data service.
 * \param randomsock        Pointer to the socket used to communicate with the
 *                          random service.
 * \param controlsock       Pointer to the socket used to control the
 *                          canonization service.
 * \param canonizationpid   Pointer to the canonization service pid, to be
 *                          updated on the successful completion of this
 *                          function.
 * \param runsecure         Set to false if we are not being run in secure mode.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED if
 *        spawning this process failed because the user is not root and
 *        runsecure is true.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_SOCKETPAIR_FAILURE if creating a
 *        socketpair for the protocol service process failed.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_FORK_FAILURE if forking the private
 *        process failed.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE if
 *        there was a failure looking up the configured user and group for the
 *        protocol service process.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_PRIVSEP_CHROOT_FAILURE if chrooting
 *        failed.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE if
 *        dropping privileges failed.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_PRIVSEP_SETFDS_FAILURE if setting
 *        file descriptors failed.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE if
 *        executing the private command failed.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS if
 *        the process survived execution (weird!).
 */
int start_canonization_proc(
    const bootstrap_config_t* bconf, const agent_config_t* conf, int* logsock,
    int* datasock, int* randomsock, int* controlsock, pid_t* canonizationpid,
    bool runsecure);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_CANONIZATIONSERVICE_HEADER_GUARD*/
