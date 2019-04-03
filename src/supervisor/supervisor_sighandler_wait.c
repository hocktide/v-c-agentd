/**
 * \file supervisor/supervisor_sighandler_wait.c
 *
 * \brief Wait for an interesting signal to occur.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>
#include <signal.h>

#include "supervisor_private.h"

/**
 * \brief Wait until a signal occurs.
 */
void supervisor_sighandler_wait()
{
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGHUP);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    sigsuspend(&oldmask);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}
