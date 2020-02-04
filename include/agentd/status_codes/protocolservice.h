/**
 * \file agentd/status_codes/protocolservice.h
 *
 * \brief Status code definitions for the protocol service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_PROTOCOLSERVICE_HEADER_GUARD
#define AGENTD_STATUS_CODES_PROTOCOLSERVICE_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Setting the protocol service process socket to non-blocking failed.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x0001U)

/**
 * \brief Adding the protocol service socket to the event loop failed.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_ADD_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x0002U)

/**
 * \brief Running the event loop failed.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_RUN_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x0003U)

/**
 * \brief The agentd process must be run as root in secure mode.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x0004U)

/**
 * \brief A socket pair could not be created.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_IPC_SOCKETPAIR_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x0005U)

/**
 * \brief Forking the protocol service process failed.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_FORK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x0006U)

/**
 * \brief Attempting to look up the configured user and group failed.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x0007U)

/**
 * \brief Attempting to change the root directory failed.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_PRIVSEP_CHROOT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x0008U)

/**
 * \brief Attempting to drop privileges failed.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x0009U)

/**
 * \brief Attempting to set file descriptors failed.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_PRIVSEP_SETFDS_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x000AU)

/**
 * \brief Attempting to execute the protocol service private command failed.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x000BU)

/**
 * \brief Somehow, we managed to survive exec.  Weird.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x000CU)

/**
 * \brief Initializing the event loop failed.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x000DU)

/**
 * \brief A malformed request was encountered.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x000EU)

/**
 * \brief A malformed response was encountered.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_RESPONSE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x000FU)

/**
 * \brief The entity is not authorized for this operation.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x0010U)

/**
 * \brief An error occurred when attempting to get random data.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_PRNG_REQUEST_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x0011U)

/**
 * \brief An error occurred when attempting to verify a transaction.
 */
#define AGENTD_ERROR_PROTOCOLSERVICE_TRANSACTION_VERIFICATION \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROTOCOL, 0x0012U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_PROTOCOLSERVICE_HEADER_GUARD*/
