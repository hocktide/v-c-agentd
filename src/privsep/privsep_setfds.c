/**
 * \file privsep/privsep_setfds.c
 *
 * \brief Set the file descriptors for a new process.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/privsep.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>

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
 *          - AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_STDIN_CLOSE if closing
 *            standard input fails.
 *          - AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_STDOUT_CLOSE if closing
 *            standard output fails.
 *          - AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_STDERR_CLOSE if closing
 *            standard error fails.
 *          - AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_DUP2_FAILURE if setting a file
 *            descriptor fails.
 *          - AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_BAD_ARGUMENT if a bad argument
 *            is encountered.
 */
int privsep_setfds(int curr, int mapped, ...)
{
    int retval = 1;
    va_list fd_list;

    /* start the fd list. */
    va_start(fd_list, mapped);

    /* replace each descriptor. */
    for (;;)
    {
        /* map the file descriptor to the new place. */
        retval = dup2(curr, mapped);
        if (retval < 0)
        {
            retval = AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_DUP2_FAILURE;
            goto done;
        }

        /* attempt to read the next descriptor. */
        curr = va_arg(fd_list, int);
        if (curr < 0)
        {
            /* a negative number denotes the end of the list. */
            retval = AGENTD_STATUS_SUCCESS;
            goto done;
        }

        /* attempt to read the next mapped descriptor. */
        mapped = va_arg(fd_list, int);
        if (mapped < 0)
        {
            /* the caller fenceposted the arguments. Error out. */
            retval = AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_BAD_ARGUMENT;
            goto done;
        }
    }

    /* success */
    retval = AGENTD_STATUS_SUCCESS;

done:
    va_end(fd_list);

    return retval;
}
