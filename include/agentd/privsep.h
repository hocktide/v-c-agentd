/**
 * \file agentd/privsep.h
 *
 * \brief Privilege separation support.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_PRIVSEP_HEADER_GUARD
#define AGENTD_PRIVSEP_HEADER_GUARD

#include <grp.h>
#include <pwd.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Get the user and group IDs by name.
 *
 * \param user          The user name to look up.
 * \param group         The group name to look up.
 * \param uid           Pointer to the user ID to set.
 * \param gid           Pointer to the group ID to set.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_GENERAL_GETPWNAM_FAILURE if user entry lookup fails.
 *          - AGENTD_ERROR_GENERAL_GETGRNAM_FAILURE if group entry lookup fails.
 */
int privsep_lookup_usergroup(
    const char* user, const char* group, uid_t* uid, gid_t* gid);

/**
 * \brief Change the root directory.
 *
 * Must be root.
 *
 * \param dir           The new root directory.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_CHDIR_FAILURE if changing to dir fails.
 *      - AGENTD_ERROR_GENERAL_CHROOT_FAILURE if changing root to dir fails.
 */
int privsep_chroot(const char* dir);

/**
 * \brief Assume the given user and group, dropping root privileges.
 *
 * Must be root.
 *
 * \param user          The user ID to assume.
 * \param group         The group ID to assume.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_PRIVSEP_DROP_PRIVILEGES_SETEGID_FAILURE if the
 *        effective group ID could not be set.
 *      - AGENTD_ERROR_GENERAL_PRIVSEP_DROP_PRIVILEGES_SETEUID_FAILURE if the
 *        effective user ID could not be set.
 *      - AGENTD_ERROR_GENERAL_PRIVSEP_DROP_PRIVILEGES_SETGID_FAILURE if the
 *        group ID could not be set.
 *      - AGENTD_ERROR_GENERAL_PRIVSEP_DROP_PRIVILEGES_SETUID_FAILURE if the
 *        user ID could not be set.
 */
int privsep_drop_privileges(uid_t user, gid_t gid);

/**
 * \brief Execute a private command.
 *
 * \param command       The private command to execute.
 *
 * \returns An error code on failure.  This method not return on success;
 * instead, the process is replaced.
 *      - AGENTD_ERROR_GENERAL_PRIVSEP_EXEC_PRIVATE_SETENV_FAILURE is returned
 *        when attempting to set the PATH / LD_LIBRARY_PATH variables fails.
 *      - AGENTD_ERROR_GENERAL_PRIVSEP_EXEC_PRIVATE_EXECL_FAILURE is returned
 *        when the execl call fails to start the private command.
 */
int privsep_exec_private(const char* command);

/**
 * \brief Close standard file descriptors.
 *
 * This method also closes all standard descriptors, such as standard in,
 * standard out, and standard error.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_STDIN_CLOSE if closing
 *            standard input fails.
 *          - AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_STDOUT_CLOSE if closing
 *            standard output fails.
 *          - AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_STDERR_CLOSE if closing
 *            standard error fails.
 */
int privsep_close_standard_fds();

/**
 * \brief Set file descriptors for a new process.
 *
 * Descriptors are described in pairs.  The first descriptor is the current
 * descriptor, and the second descriptor is the descriptor that the first is
 * mapped to in the new process.  A negative value ends this sequence.  A
 * negative value must be the last value in this sequence to act as a sentry
 * value.
 *
 * \param curr          The current descriptor.
 * \param mapped        The mapped descriptor.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_DUP2_FAILURE if setting a file
 *            descriptor fails.
 *          - AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_BAD_ARGUMENT if a bad argument
 *            is encountered.
 */
int privsep_setfds(int curr, int mapped, ...);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_PRIVSEP_HEADER_GUARD*/
