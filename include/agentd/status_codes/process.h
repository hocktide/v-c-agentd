/**
 * \file agentd/status_codes/process.h
 *
 * \brief Status code definitions for the process pseudo-service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_PROCESS_HEADER_GUARD
#define AGENTD_STATUS_CODES_PROCESS_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief The process was already spawned.
 */
#define AGENTD_ERROR_PROCESS_ALREADY_SPAWNED \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROCESS, 0x0001U)

/**
 * \brief Attempt to stop a process that wasn't running.
 */
#define AGENTD_ERROR_PROCESS_NOT_ACTIVE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_PROCESS, 0x0002U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_PROCESS_HEADER_GUARD*/
