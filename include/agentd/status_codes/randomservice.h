/**
 * \file agentd/status_codes/randomservice.h
 *
 * \brief Status code definitions for randomservice.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_RANDOMSERVICE_HEADER_GUARD
#define AGENTD_STATUS_CODES_RANDOMSERVICE_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Creating a new randomservice instance failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_INSTANCE_CREATE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0001U)

/**
 * \brief Setting the a randomservice socket to non-blocking failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_IPC_MAKE_NOBLOCK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0002U)

/**
 * \brief Initializing the event loop failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_IPC_EVENT_LOOP_INIT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0003U)

/**
 * \brief Adding the randomservice socket to the event loop failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_IPC_EVENT_LOOP_ADD_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0004U)

/**
 * \brief Running the event loop failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_IPC_EVENT_LOOP_RUN_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0005U)

/**
 * \brief Invalid request packet size.
 */
#define AGENTD_ERROR_RANDOMSERVICE_REQUEST_PACKET_INVALID_SIZE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0006U)

/**
 * \brief A bad request packet was encountered.
 */
#define AGENTD_ERROR_RANDOMSERVICE_REQUEST_PACKET_BAD \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0007U)

/**
 * \brief Error writing data to the socket.
 */
#define AGENTD_ERROR_RANDOMSERVICE_IPC_WRITE_DATA_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0008U)

/**
 * \brief Error reading data from the socket.
 */
#define AGENTD_ERROR_RANDOMSERVICE_IPC_READ_DATA_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0009U)

/**
 * \brief The requested size for get random bytes is invalid.
 */
#define AGENTD_ERROR_RANDOMSERVICE_GET_RANDOM_BYTES_INVALID_SIZE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x000AU)

/**
 * \brief The read from the PRNG failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_GET_RANDOM_BYTES_READ_FAILED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x000BU)

/**
 * \brief The agentd process must be run as root in secure mode.
 */
#define AGENTD_ERROR_RANDOMSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x000CU)

/**
 * \brief Forking the protocol service process failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_FORK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x000DU)

/**
 * \brief Opening the random device failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_RANDOM_DEVICE_OPEN_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x000EU)

/**
 * \brief Attempting to look up the configured user and group failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x000FU)

/**
 * \brief Attempting to change the root directory failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_CHROOT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0010U)

/**
 * \brief Attempting to drop privileges failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0011U)

/**
 * \brief A socket pair could not be created.
 */
#define AGENTD_ERROR_RANDOMSERVICE_IPC_SOCKETPAIR_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0012U)

/**
 * \brief Attempting to set file descriptors failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_SETFDS_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0013U)

/**
 * \brief Attempting to execute the random service private command failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0014U)

/**
 * \brief Somehow, we managed to survive exec.  Weird.
 */
#define AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0015U)

/**
 * \brief Attempting to close all other FDs failed.
 */
#define AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_CLOSE_OTHER_FDS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_RANDOM, 0x0016U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_RANDOMSERVICE_HEADER_GUARD*/
