/**
 * \file supervisor/supervisor_proc.c
 *
 * \brief Spawn the actual supervisor process.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <vpr/parameters.h>
#include <vpr/parameters.h>

#include <agentd/config.h>
#include <agentd/fds.h>
#include <agentd/ipc.h>
#include <agentd/privsep.h>
#include <agentd/bootstrap_config.h>
#include <agentd/supervisor.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>

static void private_signal_handler_forwarder(int signal);

static pid_t pid;

int supervisor_proc(struct bootstrap_config* bconf, int pid_fd)
{
    int retval = 0;

    /* verify that this process is running as root. */
    if (0 != geteuid())
    {
        fprintf(stderr, "agentd must be run as root.\n");
        retval = 1;
        goto done;
    }

    /* fork the process. */
    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        retval = pid;
        goto done;
    }

    /* Child */
    if (pid == 0)
    {
        /* If we arn't running in foreground we need a new supervision id. */
        if (!bconf->foreground)
        {
            pid_t UNUSED(sid) = setsid();
        }

        /* TODO - make it possible to chroot here by replicating user info for
         * supervisor process. */
        /* change into the prefix directory. */
        retval = chdir(bconf->prefix_dir);
        if (0 != retval)
        {
            perror("chdir");
            goto done;
        }

        /* Child holds the pid_fd lock. */
        if (flock(pid_fd, LOCK_EX | LOCK_NB) < 0)
        {
            retval = 2;
            goto done;
        }

        /* Get the child pid, write it to the pid file. */
        pid_t child_pid = getpid();
        FILE* fp = fdopen(dup(pid_fd), "w");

        if (fp == NULL)
        {
            close(pid_fd);
            perror("fdopen pid_fd");
            goto done;
        }

        fprintf(fp, "%d", child_pid);
        fclose(fp);

        /* close standard file descriptors */
        retval = privsep_close_standard_fds();
        if (0 != retval)
        {
            perror("privsep_close_standard_fds");
            goto done;
        }

        retval = privsep_setfds(
            pid_fd, /* ==> */ AGENTD_FD_PID,
            -1);
        if (0 != retval)
        {
            perror("privsep_setfds");
            goto done;
        }

        /* If successful does _not_ return! */
        retval = execl(bconf->binary, bconf->binary, "-P", "supervisor", NULL);
        if (0 != retval)
        {
            perror("privsep_exec_private");
            goto done;
        }
    }
    else
    {
        if (bconf->foreground)
        {
            /* install signal handlers. */
            signal(SIGHUP, private_signal_handler_forwarder);
            signal(SIGKILL, private_signal_handler_forwarder);
            signal(SIGTERM, private_signal_handler_forwarder);
            signal(SIGCHLD, private_signal_handler_forwarder);

            int pid_status;
            waitpid(pid, &pid_status, 0);

            /* Use the return value of the child process as our return value. */
            if (WIFEXITED(pid_status))
            {
                retval = WEXITSTATUS(pid_status);
            }
            else
            {
                retval = 1;
            }
        }
    }

done:
    return retval;
}

/**
 * Forward signal to child process if running in the foreground.
 *
 * /param signal        The signal to forward.
 */
static void private_signal_handler_forwarder(int signal)
{
    if (signal == SIGCHLD)
    {
        wait(NULL);
    }
    else
    {
        kill(pid, signal);
    }
}
