/**
 * \file privsep/privsep_close_other_fds.c
 *
 * \brief Close all file descriptors greater than the file descriptor argument.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/privsep.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>

/**
 * \brief Close any descriptors greater than the given descriptor.
 *
 * \param fd            Any descriptor greater than this descriptor and less
 *                      than or equal to FD_SETSIZE will be closed.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int privsep_close_other_fds(int fd)
{
    for (int i = fd + 1; i <= FD_SETSIZE; ++i)
    {
        close(i);
    }

    return AGENTD_STATUS_SUCCESS;
}
