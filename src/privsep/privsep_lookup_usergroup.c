/**
 * \file privsep/privsep_lookup_usergroup.c
 *
 * \brief Look up the user and group by name.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/privsep.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

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
    const char* user, const char* group, uid_t* uid, gid_t* gid)
{
    struct passwd* pwd = NULL;
    struct group* grp = NULL;

    MODEL_ASSERT(NULL != user);
    MODEL_ASSERT(NULL != group);
    MODEL_ASSERT(NULL != uid);
    MODEL_ASSERT(NULL != gid);

    /* get the nobody user. */
    pwd = getpwnam(user);
    if (NULL == pwd)
    {
        return AGENTD_ERROR_GENERAL_GETPWNAM_FAILURE;
    }

    /* get the nobody group. */
    grp = getgrnam(group);
    if (NULL == grp)
    {
        return AGENTD_ERROR_GENERAL_GETGRNAM_FAILURE;
    }

    /* assign the user and group IDs. */
    *uid = pwd->pw_uid;
    *gid = grp->gr_gid;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
