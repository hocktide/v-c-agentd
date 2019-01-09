/**
 * \file agentd/status_codes/dataservice.h
 *
 * \brief Status code definitions for dataservice.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_DATASERVICE_HEADER_GUARD
#define AGENTD_STATUS_CODES_DATASERVICE_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Not found error.
 */
#define AGENTD_ERROR_DATASERVICE_NOT_FOUND \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0001U)

/**
 * \brief Not authorized error.
 */
#define AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0003U)

/**
 * \brief Error reading data from the socket.
 */
#define AGENTD_ERROR_DATASERVICE_IPC_READ_DATA_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0004U)

/**
 * \brief The size of the API response packet was invalid.
 */
#define AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_DATA_PACKET_SIZE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0005U)

/**
 * \brief The method code of the API response packet was unexpected.
 */
#define AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0006U)

/**
 * \brief The payload data was malformed.
 */
#define AGENTD_ERROR_DATASERVICE_RECVRESP_MALFORMED_PAYLOAD_DATA \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0007U)

/**
 * \brief Error writing data to the socket.
 */
#define AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0008U)

/**
 * \brief Block Make constraint violation on block height.
 */
#define AGENTD_ERROR_DATASERVICE_BLOCK_MAKE_CONSTRAINT_BLOCK_HEIGHT \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0009U)

/**
 * \brief Block Make constraint violation on previous block UUID.
 */
#define AGENTD_ERROR_DATASERVICE_BLOCK_MAKE_CONSTRAINT_PREVIOUS_BLOCK_UUID \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x000AU)

/**
 * \brief Block Make constraint violation on block UUID.
 */
#define AGENTD_ERROR_DATASERVICE_BLOCK_MAKE_CONSTRAINT_BLOCK_UUID \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x000BU)

/**
 * \brief Block Make constraint violation: no child transactions.
 */
#define AGENTD_ERROR_DATASERVICE_BLOCK_MAKE_CONSTRAINT_NO_CHILD_TRANSACTIONS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x000CU)

/**
 * \brief Block Make block insertion failure.
 */
#define AGENTD_ERROR_DATASERVICE_BLOCK_MAKE_BLOCK_INSERTION_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x000EU)

/**
 * \brief Block Make child transaction processing failure.
 */
#define AGENTD_ERROR_DATASERVICE_BLOCK_MAKE_CHILD_TRANSACTION_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x000FU)

/**
 * \brief Invalid request packet size.
 */
#define AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0010U)

/**
 * \brief Bad child context index.
 */
#define AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_BAD_INDEX \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0011U)

/**
 * \brief Invalid child context.
 */
#define AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_INVALID \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0012U)

/**
 * \brief The maximum number of child contexts have already been created.
 */
#define AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_MAX_REACHED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0013U)

/**
 * \brief A bad request packet was encountered.
 */
#define AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_BAD \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0014U)

/**
 * \brief The agentd process must be run as root in secure mode.
 */
#define AGENTD_ERROR_DATASERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0015U)

/**
 * \brief A socket pair could not be created.
 */
#define AGENTD_ERROR_DATASERVICE_IPC_SOCKETPAIR_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0016U)

/**
 * \brief Forking the dataservice process failed.
 */
#define AGENTD_ERROR_DATASERVICE_FORK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0017U)

/**
 * \brief Attempting to look up the configured user and group failed.
 */
#define AGENTD_ERROR_DATASERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0018U)

/**
 * \brief Attempting to change the root directory failed.
 */
#define AGENTD_ERROR_DATASERVICE_PRIVSEP_CHROOT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0019U)

/**
 * \brief Attempting to drop privileges failed.
 */
#define AGENTD_ERROR_DATASERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x001AU)

/**
 * \brief Attempting to set file descriptors failed.
 */
#define AGENTD_ERROR_DATASERVICE_PRIVSEP_SETFDS_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x001BU)

/**
 * \brief Attempting to execute the dataservice private command failed.
 */
#define AGENTD_ERROR_DATASERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x001CU)

/**
 * \brief Somehow, we managed to survive exec.  Weird.
 */
#define AGENTD_ERROR_DATASERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x001DU)

/**
 * \brief Attempting to close the child context failed.
 */
#define AGENTD_ERROR_DATASERVICE_CHILD_DETAILS_DELETE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x001EU)

/**
 * \brief Creating a new dataservice instance failed.
 */
#define AGENTD_ERROR_DATASERVICE_INSTANCE_CREATE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x001FU)

/**
 * \brief Setting the dataservice process socket to non-blocking failed.
 */
#define AGENTD_ERROR_DATASERVICE_IPC_MAKE_NOBLOCK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0020U)

/**
 * \brief Initializing the event loop failed.
 */
#define AGENTD_ERROR_DATASERVICE_IPC_EVENT_LOOP_INIT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0021U)

/**
 * \brief Adding the dataservice socket to the event loop failed.
 */
#define AGENTD_ERROR_DATASERVICE_IPC_EVENT_LOOP_ADD_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0022U)

/**
 * \brief Running the event loop failed.
 */
#define AGENTD_ERROR_DATASERVICE_IPC_EVENT_LOOP_RUN_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0023U)

/**
 * \brief Creating a child context failed.
 */
#define AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_CREATE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_DATASERVICE, 0x0024U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_DATASERVICE_HEADER_GUARD*/
