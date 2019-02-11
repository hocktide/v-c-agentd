/**
 * \file agentd/status_codes.h
 *
 * \brief Status code definitions for agentd.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_HEADER_GUARD
#define AGENTD_STATUS_CODES_HEADER_GUARD

#include <agentd/services.h>
#include <agentd/status_codes/config.h>
#include <agentd/status_codes/dataservice.h>
#include <agentd/status_codes/general.h>
#include <agentd/status_codes/ipc.h>
#include <agentd/status_codes/protocolservice.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Success status code.
 */
#define AGENTD_STATUS_SUCCESS 0x00000000

/**
 * \brief Error code macro.
 */
#define AGENTD_STATUS_ERROR_MACRO(service, reason) \
    ((int32_t)(0x8000000 | ((service & 0xFF) << 16) | (reason & 0xFFFF)))

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_HEADER_GUARD*/
