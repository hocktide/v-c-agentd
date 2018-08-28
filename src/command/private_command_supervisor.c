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

    signal(SIGHUP, private_signal_handler_supervisor);
    signal(SIGKILL, private_signal_handler_supervisor);
    signal(SIGTERM, private_signal_handler_supervisor);
    signal(SIGCHLD, private_signal_handler_supervisor);

    for (;;)
    {
        sleep(10);
    }
}

static void private_signal_handler_supervisor(int signal)
{
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
