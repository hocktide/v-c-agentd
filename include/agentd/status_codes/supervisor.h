/**
 * \file agentd/status_codes/supervisor.h
 *
 * \brief Status code definitions for the supervisor service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_SUPERVISOR_HEADER_GUARD
#define AGENTD_STATUS_CODES_SUPERVISOR_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief The supervisor failed to install a signal handler.
 */
#define AGENTD_ERROR_SUPERVISOR_SIGNAL_INSTALLATION \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_SUPERVISOR, 0x0001U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_SUPERVISOR_HEADER_GUARD*/
