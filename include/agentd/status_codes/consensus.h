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

/**
 * \brief Error writing data to the socket.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_IPC_WRITE_DATA_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x000BU)

/**
 * \brief Error reading data from the socket.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_IPC_READ_DATA_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x000CU)

/**
 * \brief Creating a new consensus service instance failed.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_INSTANCE_CREATE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x000DU)

/**
 * \brief Setting a socket to non-blocking failed.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_IPC_MAKE_NOBLOCK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x000EU)

/**
 * \brief Adding a socket to the event loop failed.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_IPC_EVENT_LOOP_ADD_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x000FU)

/**
 * \brief The request packet was an invalid size.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_REQUEST_PACKET_INVALID_SIZE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0010U)

/**
 * \brief A bad request packet was encountered.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_REQUEST_PACKET_BAD \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0011U)

/**
 * \brief The response packet was an invalid size.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_RESPONSE_PACKET_INVALID_SIZE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0012U)

/**
 * \brief An unexpected method code was encountered in the response.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_RECVRESP_UNEXPECTED_METHOD_CODE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0013U)

/**
 * \brief An attempt was made to start the consensus service before it was
 * configured.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_START_BEFORE_CONFIGURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0014U)

/**
 * \brief The consensus service is already running and can't be started again.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_ALREADY_RUNNING \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0015U)

/**
 * \brief A bad parameter was passed to a consensus service API call.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_BAD_PARAMETER \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0016U)

/**
 * \brief The consensus service was already configured.
 */
#define AGENTD_ERROR_CONSENSUSSERVICE_ALREADY_CONFIGURED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_CONSENSUS, 0x0017U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_CONSENSUS_HEADER_GUARD*/
