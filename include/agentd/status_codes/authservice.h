/**
 * \file agentd/status_codes/authservice.h
 *
 * \brief Status code definitions for the auth service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_AUTHSERVICE_HEADER_GUARD
#define AGENTD_STATUS_CODES_AUTHSERVICE_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Setting the auth service process socket to non-blocking failed.
 */
#define AGENTD_ERROR_AUTHSERVICE_IPC_MAKE_NOBLOCK_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_AUTH, 0x0001U)

/**
 * \brief Adding the protocol service socket to the event loop failed.
 */
#define AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_ADD_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_AUTH, 0x0002U)


/**
 * \brief Running the event loop failed.
 */
#define AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_RUN_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_AUTH, 0x0003U)


/**
 * \brief Initializing the event loop failed.
 */
#define AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_INIT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_AUTH, 0x0004U)


/**
 * \brief The size of the API response packet was invalid.
 */
#define AGENTD_ERROR_AUTHSERVICE_REQUEST_PACKET_INVALID_SIZE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_AUTH, 0x0005U)


/**
 * \brief A bad request packet was encountered.
 */
#define AGENTD_ERROR_AUTHSERVICE_REQUEST_PACKET_BAD \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_AUTH, 0x0006U)


/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_AUTHSERVICE_HEADER_GUARD*/
