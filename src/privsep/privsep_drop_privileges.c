/**
 * \file privsep/privsep_drop_privileges.c
 *
 * \brief Drop privileges to the given user and group IDs.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/privsep.h>
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
 * \returns 0 on success and non-zero on failure.
 */
int privsep_drop_privileges(uid_t uid, gid_t gid)
{
    int retval = 1;

    /* set the effective group ID. */
    retval = setegid(gid);
    if (0 != retval)
    {
        return retval;
    }

    /* set the effective user ID. */
    retval = seteuid(uid);
    if (0 != retval)
    {
        return retval;
    }

    /* set the group ID. */
    retval = setgid(gid);
    if (0 != retval)
    {
        return retval;
    }

    /* set the user ID. */
    retval = setuid(uid);
    if (0 != retval)
    {
        return retval;
    }

    return 0;
}
