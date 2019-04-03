/**
 * \file supervisor/supervisor_signal_handler.c
 *
 * \brief Signal handler for the supervisor process.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>
#include <signal.h>

#include "supervisor_private.h"

/* flag to indicate whether we should continue running. */
bool keep_running = false;

/**
 * \brief Signal handler for the supervisor process.
 */
void supervisor_signal_handler(int signal)
{
    switch (signal)
    {
        case SIGCHLD:
        case SIGHUP:
            /* restart */
            break;

        case SIGTERM:
        default:
            /* attempt to gracefully shut down the process. */
            keep_running = false;
            break;
    }
}
