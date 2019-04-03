/**
 * \file supervisor/supervisor_sighandler_install.c
 *
 * \brief Install the signal handler for the supervisor.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>
#include <signal.h>

#include "supervisor_private.h"

/**
 * \brief Install the signal handler for the supervisor.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_sighandler_install()
{
    /* attempt to catch SIGHUP signal. */
    if (SIG_ERR == signal(SIGHUP, &supervisor_signal_handler))
    {
        return AGENTD_ERROR_SUPERVISOR_SIGNAL_INSTALLATION;
    }

    /* attempt to catch SIGTERM signal. */
    if (SIG_ERR == signal(SIGTERM, &supervisor_signal_handler))
    {
        return AGENTD_ERROR_SUPERVISOR_SIGNAL_INSTALLATION;
    }

    /* attempt to catch SIGCHLD signal. */
    if (SIG_ERR == signal(SIGCHLD, &supervisor_signal_handler))
    {
        return AGENTD_ERROR_SUPERVISOR_SIGNAL_INSTALLATION;
    }

    /* success */
    return AGENTD_STATUS_SUCCESS;
}
