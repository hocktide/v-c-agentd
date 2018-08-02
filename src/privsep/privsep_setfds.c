/**
 * \file privsep/privsep_setfds.c
 *
 * \brief Set the file descriptors for a new process.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/privsep.h>
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
 * This method also closes all standard descriptors, such as standard in,
 * standard out, and standard error.
 *
 * \param curr          The current descriptor.
 * \param mapped        The mapped descriptor.
 *
 * \returns Zero on success and non-zero on failure.
 */
int privsep_setfds(int curr, int mapped, ...)
{
    int retval = 1;
    va_list fd_list;

    /* start the fd list. */
    va_start(fd_list, mapped);

    /* close standard in */
    retval = close(STDIN_FILENO);
    if (0 != retval)
    {
        goto done;
    }

    /* close standard out. */
    retval = close(STDOUT_FILENO);
    if (0 != retval)
    {
        goto done;
    }

    /* close standard error. */
    retval = close(STDERR_FILENO);
    if (0 != retval)
    {
        goto done;
    }

    /* replace each descriptor. */
    for (;;)
    {
        /* map the file descriptor to the new place. */
        retval = dup2(curr, mapped);
        if (0 > retval)
        {
            goto done;
        }

        /* attempt to read the next descriptor. */
        curr = va_arg(fd_list, int);
        if (0 > curr)
        {
            retval = 0;
            goto done;
        }

        /* attempt to read the next mapped descriptor. */
        mapped = va_arg(fd_list, int);
        if (0 > mapped)
        {
            /* the caller fenceposted the arguments. Error out. */
            retval = 1;
            goto done;
        }
    }

    /* success */
    retval = 0;

done:
    va_end(fd_list);

    return retval;
}
