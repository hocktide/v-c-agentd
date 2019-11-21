/**
 * \file agentd/status_codes/consensus.h
 *
 * \brief Status code definitions for the consensus service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_CONSENSUS_HEADER_GUARD
#define AGENTD_STATUS_CODES_CONSENSUS_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief The agentd process must be run as root in secure mode.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0001U)

/**
 * \brief Forking the consensus service process failed.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_FORK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0002U)

/**
 * \brief Attempting to look up the configured user and group failed.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0003U)

/**
 * \brief Attempting to change the root directory failed.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_PRIVSEP_CHROOT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0004U)

/**
 * \brief Attempting to drop privileges failed.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0005U)

/**
 * \brief Attempting to set file descriptors failed.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_PRIVSEP_SETFDS_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0006U)

/**
 * \brief Attempting to execute the consensus service private command failed.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0007U)

/**
 * \brief Somehow, we managed to survive exec.  Weird.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0008U)

/**
 * \brief Error initializing the event loop.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_INIT \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0009U)

/**
 * \brief Error running the event loop.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_RUN_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x000AU)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_CONSENSUS_HEADER_GUARD*/
