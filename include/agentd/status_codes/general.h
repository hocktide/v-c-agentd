/**
 * \file agentd/status_codes/general.h
 *
 * \brief Status code definitions for general functions.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STATUS_CODES_GENERAL_HEADER_GUARD
#define AGENTD_STATUS_CODES_GENERAL_HEADER_GUARD

#include <agentd/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Out of memory error.
 */
#define AGENTD_ERROR_GENERAL_OUT_OF_MEMORY \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x0001U)

/**
 * \brief The requested binary was not found in the current path.
 */
#define AGENTD_ERROR_GENERAL_PATH_NOT_FOUND \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x0002U)

/**
 * \brief The requested change directory operation failed.
 */
#define AGENTD_ERROR_GENERAL_CHDIR_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x0003U)

/**
 * \brief The requested change root directory operation failed.
 */
#define AGENTD_ERROR_GENERAL_CHROOT_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x0004U)

/**
 * \brief Getting the password file entry for the given user failed.
 */
#define AGENTD_ERROR_GENERAL_GETPWNAM_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x0005U)

/**
 * \brief Getting the group file entry for the given group failed.
 */
#define AGENTD_ERROR_GENERAL_GETGRNAM_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x0006U)

/**
 * \brief Closing stdin during privsep_setfds failed.
 */
#define AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_STDIN_CLOSE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x0007U)

/**
 * \brief Closing stdout during privsep_setfds failed.
 */
#define AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_STDOUT_CLOSE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x0008U)

/**
 * \brief Closing stderr during privsep_setfds failed.
 */
#define AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_STDERR_CLOSE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x0009U)

/**
 * \brief Attempting to set the file descriptor using dup2 failed.
 */
#define AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_DUP2_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x000AU)

/**
 * \brief Bad argument to privsep_setfds.
 */
#define AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_BAD_ARGUMENT \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x000BU)

/**
 * \brief When dropping privileges, setegid failed.
 */
#define AGENTD_ERROR_GENERAL_PRIVSEP_DROP_PRIVILEGES_SETEGID_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x000CU)

/**
 * \brief When dropping privileges, seteuid failed.
 */
#define AGENTD_ERROR_GENERAL_PRIVSEP_DROP_PRIVILEGES_SETEUID_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x000DU)

/**
 * \brief When dropping privileges, setgid failed.
 */
#define AGENTD_ERROR_GENERAL_PRIVSEP_DROP_PRIVILEGES_SETGID_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x000EU)

/**
 * \brief When dropping privileges, setuid failed.
 */
#define AGENTD_ERROR_GENERAL_PRIVSEP_DROP_PRIVILEGES_SETUID_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x000FU)

/**
 * \brief When executing a private command, setenv failed.
 */
#define AGENTD_ERROR_GENERAL_PRIVSEP_EXEC_PRIVATE_SETENV_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x0010U)

/**
 * \brief When executing a private command, execl failed.
 */
#define AGENTD_ERROR_GENERAL_PRIVSEP_EXEC_PRIVATE_EXECL_FAILURE \
    AGENTD_STATUS_ERROR_MACRO(AGENTD_SERVICE_GENERAL, 0x0011U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STATUS_CODES_GENERAL_HEADER_GUARD*/
