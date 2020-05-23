/**
 * \file agentd/status_codes/ipc.h
 *
 * \brief Status code definitions for IPC functions.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_IPC_HEADER_GUARD
#define AGENTD_STATUS_CODES_IPC_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief The given operation was halted because it would block.
 */
#define AGENTD_ERROR_IPC_WOULD_BLOCK \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0001U)

/**
 * \brief A blocking read failed.
 */
#define AGENTD_ERROR_IPC_READ_BLOCK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0002U)

/**
 * \brief A blocking write failed.
 */
#define AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0003U)

/**
 * \brief A non-blocking read failed.
 */
#define AGENTD_ERROR_IPC_READ_NONBLOCK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0004U)

/**
 * \brief A non-blocking write failed.
 */
#define AGENTD_ERROR_IPC_WRITE_NONBLOCK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0005U)

/**
 * \brief A failure occurred when calling event_base_new.
 */
#define AGENTD_ERROR_IPC_EVENT_BASE_NEW_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0006U)

/**
 * \brief The data type read from the stream was unexpected.
 */
#define AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0007U)

/**
 * \brief The data size read from the stream was unexpected.
 */
#define AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0008U)

/**
 * \brief There was an error draining the read buffer.
 */
#define AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0009U)

/**
 * \brief There was an error removing data from the read buffer.
 */
#define AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x000AU)

/**
 * \brief There was an error adding type data to the write buffer.
 */
#define AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x000BU)

/**
 * \brief There was an error adding size data to the write buffer.
 */
#define AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x000CU)

/**
 * \brief There was an error adding payload data to the write buffer.
 */
#define AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x000DU)

/**
 * \brief There was an error getting fcntl flags.
 */
#define AGENTD_ERROR_IPC_FCNTL_GETFL_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x000EU)

/**
 * \brief There was an error setting fcntl flags.
 */
#define AGENTD_ERROR_IPC_FCNTL_SETFL_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x000FU)

/**
 * \brief There was an error setting up a new signal in libevent.
 */
#define AGENTD_ERROR_IPC_EVSIGNAL_NEW_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0010U)

/**
 * \brief There was an adding an event to libevent.
 */
#define AGENTD_ERROR_IPC_EVENT_ADD_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0011U)

/**
 * \brief The provided argument was invalid.
 */
#define AGENTD_ERROR_IPC_INVALID_ARGUMENT \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0012U)

/**
 * \brief A callback is required for this call but was not provided.
 */
#define AGENTD_ERROR_IPC_MISSING_CALLBACK \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0013U)

/**
 * \brief An attempt to create a new event buffer failed.
 */
#define AGENTD_ERROR_IPC_EVBUFFER_NEW_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0014U)

/**
 * \brief An attempt to create a new event failed.
 */
#define AGENTD_ERROR_IPC_EVENT_NEW_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0015U)

/**
 * \brief An attempt to read from a socket into a buffer failed.
 */
#define AGENTD_ERROR_IPC_EVBUFFER_READ_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0016U)

/**
 * \brief An attempt to create a socketpair failed.
 */
#define AGENTD_ERROR_IPC_SOCKETPAIR_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0017U)

/**
 * \brief An attempt to accept a socket failed.
 */
#define AGENTD_ERROR_IPC_ACCEPT_NOBLOCK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0018U)

/**
 * \brief General crypto failure in IPC request.
 */
#define AGENTD_ERROR_IPC_CRYPTO_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x0019U)

/**
 * \brief An auth packet could not be authorized.
 */
#define AGENTD_ERROR_IPC_UNAUTHORIZED_PACKET \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x001AU)

/**
 * \brief An EOF condition was encountered reading from a socket.
 */
#define AGENTD_ERROR_IPC_EVBUFFER_EOF \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x001BU)

/**
 * \brief A Linuxism error occurred during accept and we should retry.
 */
#define AGENTD_ERROR_IPC_ACCEPT_SHOULD_RETRY \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_IPC, 0x001CU)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_IPC_HEADER_GUARD*/
