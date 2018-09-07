/**
 * \file command/command_start.c
 *
 * \brief Start the agent.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */
#include <agentd/command.h>
#include <agentd/config.h>
#include <agentd/string.h>
#include <agentd/supervisor.h>
#include <unistd.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>

/**
 * \brief Start the blockchain agent.
 *
 * \param bconf         The bootstrap configuration for this command.
 *
 * \returns 0 on success and non-zero on failure.
 */
int command_start(struct bootstrap_config* bconf)
{
    int retval = 0;
    char* pid_path = strcatv(bconf->prefix_dir, "/var/pid/agentd.pid", NULL);

    /* Create this file if it doesn't exist. */
    int pid_fd = open(
        pid_path,
        O_CREAT | O_RDWR,
        S_IRUSR | S_IWUSR);
    if (pid_fd == -1)
    {
        perror("Error creating pid file.");
        retval = 1;
        goto done;
    }

    /* Try and lock it exclusive. */
    if (flock(pid_fd, LOCK_EX | LOCK_NB) < 0)
    {
        if (flock(pid_fd, LOCK_UN | LOCK_NB) < 0)
        {
            fprintf(stderr, "Cannot release existing lock.\n");
            retval = 2;
            goto done;
        }
    }
    else
    {
        if (flock(pid_fd, LOCK_UN | LOCK_NB) < 0)
        {
            fprintf(stderr, "Cannot release my lock.\n");
            retval = 2;
            goto done;
        }
    }

    retval = supervisor_proc(bconf, pid_fd);

done:
    free(pid_path);
    close(pid_fd);

    return retval;
}
