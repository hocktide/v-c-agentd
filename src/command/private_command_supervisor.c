/**
 * \file command/private_command_supervisor.c
 *
 * \brief Create, spawn, and introduce all services.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

/* forward decls. */
static int sighandler_install();
static void sighandler_uninstall();
static void supervisor_signal_handler(int signal);
static int supervisor_run();

/* flag to indicate whether we should continue running. */
static bool keep_running = false;

/**
 * \brief Run the the supervisor.
 */
void private_command_supervisor()
{
    /* install the signal handlers. */
    if (AGENTD_STATUS_SUCCESS != sighandler_install())
    {
        perror("sighandler_install");
        return;
    }

    /* we are in the running state. */
    keep_running = true;

    /* TODO - set the process name. */

    while (keep_running)
    {
        /* if supervisor_run fails, exit. */
        if (AGENTD_STATUS_SUCCESS != supervisor_run())
        {
            keep_running = false;
        }
    }

    /* uninstall the signal handlers on exit. */
    sighandler_uninstall();
}

/**
 * \brief Run the supervisor.
 *
 * This function attempts to bootstrap all child services and then waits until
 * an appropriate signal is detected prior to exiting.
 *
 * The bootstrap process first reads the configuration file and then uses this
 * configuration file to start each child service.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 */
static int supervisor_run()
{
    int retval = AGENTD_STATUS_SUCCESS;

    /* read config. */
    /* create listener service. */
    /* create data service for protocol service. */
    /* create protocol service. */
    /* create data service for consensus service. */
    /* create consensus service. */

    sleep(10);

    return retval;
}

/**
 * \brief Install signal handlers.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_SUPERVISOR_SIGNAL_INSTALLATION if installation of a
 *            signal handler failed.
 */
static int sighandler_install()
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

/**
 * \brief Uninstall signal handlers.
 */
static void sighandler_uninstall()
{
    /* since we are on the way out, failure doesn't matter. */
    signal(SIGHUP, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
}

/**
 * \brief Signal handler for the supervisor process.
 */
static void supervisor_signal_handler(int signal)
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
