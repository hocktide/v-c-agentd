/**
 * \file agentd/status_codes/listenservice.h
 *
 * \brief Status code definitions for the listen service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_LISTENSERVICE_HEADER_GUARD
#define AGENTD_STATUS_CODES_LISTENSERVICE_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Setting the listen service process socket to non-blocking failed.
 */
#define AGENTD_ERROR_LISTENSERVICE_IPC_MAKE_NOBLOCK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x0001U)

/**
 * \brief Initializing the event loop failed.
 */
#define AGENTD_ERROR_LISTENSERVICE_IPC_EVENT_LOOP_INIT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x0002U)

/**
 * \brief Adding the listen service socket to the event loop failed.
 */
#define AGENTD_ERROR_LISTENSERVICE_IPC_EVENT_LOOP_ADD_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x0003U)

/**
 * \brief Running the event loop failed.
 */
#define AGENTD_ERROR_LISTENSERVICE_IPC_EVENT_LOOP_RUN_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x0004U)

/**
 * \brief The agentd process must be run as root in secure mode.
 */
#define AGENTD_ERROR_LISTENSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x0005U)

/**
 * \brief A socket pair could not be created.
 */
#define AGENTD_ERROR_LISTENSERVICE_IPC_SOCKETPAIR_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x0006U)

/**
 * \brief Forking the listen service process failed.
 */
#define AGENTD_ERROR_LISTENSERVICE_FORK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x0007U)

/**
 * \brief Attempting to look up the configured user and group failed.
 */
#define AGENTD_ERROR_LISTENSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x0008U)

/**
 * \brief Attempting to change the root directory failed.
 */
#define AGENTD_ERROR_LISTENSERVICE_PRIVSEP_CHROOT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x0009U)

/**
 * \brief Attempting to drop privileges failed.
 */
#define AGENTD_ERROR_LISTENSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x000AU)

/**
 * \brief Attempting to set file descriptors failed.
 */
#define AGENTD_ERROR_LISTENSERVICE_PRIVSEP_SETFDS_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x000BU)

/**
 * \brief Attempting to execute the listen service private command failed.
 */
#define AGENTD_ERROR_LISTENSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x000CU)

/**
 * \brief Somehow, we managed to survive exec.  Weird.
 */
#define AGENTD_ERROR_LISTENSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_LISTEN, 0x000DU)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_LISTENSERVICE_HEADER_GUARD*/
