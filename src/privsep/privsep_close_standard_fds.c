/**
 * \file privsep/privsep_close_standard_fds.c
 *
 * \brief Close standard file descriptors for a process.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/privsep.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>

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
int privsep_close_standard_fds()
{
    int retval = 0;

    /* close standard in */
    retval = close(STDIN_FILENO);
    if (0 != retval)
    {
        retval = AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_STDIN_CLOSE;
        goto done;
    }

    /* close standard out. */
    retval = close(STDOUT_FILENO);
    if (0 != retval)
    {
        retval = AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_STDOUT_CLOSE;
        goto done;
    }

    /* close standard error. */
    retval = close(STDERR_FILENO);
    if (0 != retval)
    {
        retval = AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_STDERR_CLOSE;
        goto done;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

done:
    return retval;
}
