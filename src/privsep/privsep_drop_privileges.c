/**
 * \file privsep/privsep_drop_privileges.c
 *
 * \brief Drop privileges to the given user and group IDs.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/privsep.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

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
int privsep_drop_privileges(uid_t uid, gid_t gid)
{
/* On OpenBSD and other Unix implementations, the effective uid / gid must be
 * adjusted first.
 */
#ifdef __OpenBSD__
    /* set the effective group ID. */
    if (0 != setegid(gid))
    {
        return AGENTD_ERROR_GENERAL_PRIVSEP_DROP_PRIVILEGES_SETEGID_FAILURE;
    }

    /* set the effective user ID. */
    if (0 != seteuid(uid))
    {
        return AGENTD_ERROR_GENERAL_PRIVSEP_DROP_PRIVILEGES_SETEUID_FAILURE;
    }
#endif /*__OpenBSD__*/

    /* set the group ID. */
    if (0 != setgid(gid))
    {
        return AGENTD_ERROR_GENERAL_PRIVSEP_DROP_PRIVILEGES_SETGID_FAILURE;
    }

    /* set the user ID. */
    if (0 != setuid(uid))
    {
        return AGENTD_ERROR_GENERAL_PRIVSEP_DROP_PRIVILEGES_SETUID_FAILURE;
    }

    return AGENTD_STATUS_SUCCESS;
}
