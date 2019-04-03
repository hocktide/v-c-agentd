/**
 * \file supervisor/supervisor_sighandler_uninstall.c
 *
 * \brief Uninstall the signal handler for the supervisor.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>
#include <signal.h>

#include "supervisor_private.h"

/**
 * \brief Uninstall signal handlers.
 */
void supervisor_sighandler_uninstall()
{
    /* since we are on the way out, failure doesn't matter. */
    signal(SIGHUP, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
}
