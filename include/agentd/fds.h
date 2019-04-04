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

/******************************************************************************/
/* Config Reader                                                              */
/******************************************************************************/

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

/******************************************************************************/
/* Data Service                                                               */
/******************************************************************************/

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

/******************************************************************************/
/* Listen Service                                                             */
/******************************************************************************/

/**
 * \brief File descriptor for the listen service log socket.
 * Used by the listen service private command.
 */
#define AGENTD_FD_LISTENSERVICE_LOG ((int)0)

/**
 * \brief File descriptor for the listen service accept socket.
 * Used by the listen service private command.
 */
#define AGENTD_FD_LISTENSERVICE_ACCEPT ((int)1)

/**
 * \brief File descriptor for the first listen socket for the listen service.
 * Used by the listen service private command.
 */
#define AGENTD_FD_LISTENSERVICE_SOCK_START ((int)2)

/******************************************************************************/
/* Supervisor Service                                                         */
/******************************************************************************/

/**
 * \brief File descriptor for the pid flocked file.
 * Used by the supervisor private command.
 */
#define AGENTD_FD_PID ((int)2)

/******************************************************************************/
/* Unauthorized Protocol Service                                              */
/******************************************************************************/

/**
 * \brief File descriptor for the unauthorized protocol service accept socket.
 * Used by the unauthorized protocol service private command.
 */
#define AGENTD_FD_UNAUTHORIZED_PROTOSVC_ACCEPT ((int)0)

/**
 * \brief File descriptor for the unauthorized protocol service log socket.
 * Used by the unauthorized protocol service private command.
 */
#define AGENTD_FD_UNAUTHORIZED_PROTOSVC_LOG ((int)1)

/**
 * \brief File descriptor for the unauthorized protocol service data socket.
 * Used by the unauthorized protocol service private command.
 */
#define AGENTD_FD_UNAUTHORIZED_PROTOSVC_DATA ((int)2)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_FDS_CONFIG_HEADER_GUARD*/
