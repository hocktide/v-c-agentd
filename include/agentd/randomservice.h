/**
 * \file agentd/randomservice.h
 *
 * \brief Service level API for the random service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_RANDOMSERVICE_HEADER_GUARD
#define AGENTD_RANDOMSERVICE_HEADER_GUARD

#include <agentd/bootstrap_config.h>
#include <agentd/config.h>
#include <agentd/protocolservice/api.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Random service API methods.
 */
enum randomservice_api_method_enum
{
    /**
     * \brief Lower bound of the API methods.  Must be the first value in this
     * enumeration.
     */
    RANDOMSERVICE_API_METHOD_LOWER_BOUND,

    /**
     * \brief Get random bytes.
     */
    RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES,

    /**
     * \brief The number of methods in this API.
     *
     * Must be immediately after the last enumerated value.
     */
    RANDOMSERVICE_API_METHOD_UPPER_BOUND
};

/**
 * \brief Event loop for the random service.  This is the entry point for the
 * random service.  It handles the details of reacting to events
 * sent over the random service sockets.
 *
 * \param random        The random device handle.
 * \param protosock     The connection with the protocol service.  This socket
 *                      connection allows the protocol service to request random
 *                      data from the random service.
 * \param logsock       The logging service socket.  The random service logs
 *                      on this socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_RANDOMSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *            attempting to make a socket non-blocking failed.
 *          - AGENTD_ERROR_RANDOMSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_RANDOMSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if adding
 *            a socket to the event loop failed.
 *          - AGENTD_ERROR_RANDOMSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if running
 *            the random service event loop failed.
 */
int randomservice_event_loop(int random, int protosock, int logsock);

/**
 * \brief Spawn a random service process using the provided config structure and
 * logger socket.
 *
 * On success, this method sets the file descriptor pointer to the file
 * descriptors for the random service socket.  This can be used by the caller to
 * send requests to the random service and to receive responses from this
 * service.  Also, the pointer to the pid for this process is set.  This can be
 * used to signal and wait when this process should be terminated.
 *
 * \param bconf         The bootstrap configuration for this service.
 * \param conf          The configuration for this service.
 * \param logsock       Socket used to communicate with the logger.
 * \param protosock     Socket used to communicate with the protocol service.
 * \param randompid     Pointer to the random service pid, to be updated on
 *                      the successful completion of this function.
 * \param runsecure     Set to false if we are not being run in secure mode.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_RANDOMSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED if
 *        spawning this process failed because the user is not root and
 *        runsecure is true.
 *      - AGENTD_ERROR_RANDOMSERVICE_IPC_SOCKETPAIR_FAILURE if creating a
 *        socketpair for the random service process failed.
 *      - AGENTD_ERROR_RANDOMSERVICE_FORK_FAILURE if forking the private
 *        process failed.
 *      - AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE if there
 *        was a failure looking up the configured user and group for the
 *        random service process.
 *      - AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_CHROOT_FAILURE if chrooting
 *        failed.
 *      - AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE if
 *        dropping privileges failed.
 *      - AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_SETFDS_FAILURE if setting file
 *        descriptors failed.
 *      - AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE if executing
 *        the private command failed.
 *      - AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS if the
 *        process survived execution (weird!).      
 */
int randomservice_proc(
    const bootstrap_config_t* bconf, const agent_config_t* conf, int logsock,
    int* protosock, pid_t* randompid, bool runsecure);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_RANDOMSERVICE_HEADER_GUARD*/
