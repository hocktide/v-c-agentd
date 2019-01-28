/**
 * \file privsep/privsep_chroot.c
 *
 * \brief Change the root directory.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/privsep.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

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
int privsep_chroot(const char* dir)
{
    MODEL_ASSERT(NULL != dir);

    /* change into the prefix directory. */
    if (0 != chdir(dir))
    {
        return AGENTD_ERROR_GENERAL_CHDIR_FAILURE;
    }

    /* change root. */
    if (0 != chroot(dir))
    {
        return AGENTD_ERROR_GENERAL_CHROOT_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
