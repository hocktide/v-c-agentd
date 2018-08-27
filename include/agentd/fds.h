/**
 * \file agentd/fds.h
 *
 * \brief File descriptors for agentd.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_FDS_CONFIG_HEADER_GUARD
#define AGENTD_FDS_CONFIG_HEADER_GUARD

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief File descriptor for the config input file.
 * Used by the readconfig private command.
 */
#define AGENTD_FD_CONFIG_IN ((int)0)

/**
 * \brief File descriptor for the config output stream.
 * Used by the readconfig private command.
 */
#define AGENTD_FD_CONFIG_OUT ((int)1)

/**
 * \brief File descriptor for the data service socket.
 * Used by the data service private command.
 */
#define AGENTD_FD_DATASERVICE_SOCK ((int)0)

/**
 * \brief File descriptor for the data service log socket.
 * Used by the data service private command.
 */
#define AGENTD_FD_DATASERVICE_LOG ((int)1)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_FDS_CONFIG_HEADER_GUARD*/
