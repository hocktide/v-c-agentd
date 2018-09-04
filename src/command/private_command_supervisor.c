/**
 * \file command/private_command_supervisor.c
 *
 * \brief Be the supervisor that spawns everyone else.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>


static void private_signal_handler_supervisor(int signal);

void private_command_supervisor()
{
    /*
     * This one is special as the application is terminated at the exit of this handler.
     * We'll want to make some sort of a notification here.
     */

    signal(SIGHUP, private_signal_handler_supervisor);
    signal(SIGKILL, private_signal_handler_supervisor);
    signal(SIGTERM, private_signal_handler_supervisor);
    signal(SIGCHLD, private_signal_handler_supervisor);

    for (;;)
    {
        wait(NULL);
    }
}

static void private_signal_handler_supervisor(int signal)
{
    /*
     * We'll want to wait on the next child pid here,
     * or bubble this up to the loop above to wait on the proc.
     */
    switch (signal)
    {
        case SIGKILL:
            break;
        case SIGTERM:
            break;
        case SIGCHLD:
            break;
        case SIGHUP:
            break;
    }
}
