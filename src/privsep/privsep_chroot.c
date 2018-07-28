/**
 * \file privsep/privsep_chroot.c
 *
 * \brief Change the root directory.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/privsep.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

/**
 * \brief Change the root directory.
 *
 * Must be root.
 *
 * \param dir           The new root directory.
 *
 * \returns 0 on success and non-zero on failure.
 */
int privsep_chroot(const char* dir)
{
    int retval = 1;

    MODEL_ASSERT(NULL != dir);

    /* change into the prefix directory. */
    retval = chdir(dir);
    if (0 != retval)
    {
        return retval;
    }

    /* change root. */
    return chroot(dir);
}
