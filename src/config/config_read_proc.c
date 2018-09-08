/**
 * \file config/config_read_proc.c
 *
 * \brief Spawn an unprivileged process to read and verify the config file.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <agentd/config.h>
#include <agentd/fds.h>
#include <agentd/ipc.h>
#include <agentd/privsep.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Spawn a process to read config data, populating the provided config
 * structure.
 *
 * On success, a config structure is initialized with data from the config
 * reader process.  This is owned by the caller and must be disposed by calling
 * \ref dispose() when no longer needed.
 *
 * \param bconf         The bootstrap configuration used to spawn the config
 *                      reader process.
 * \param conf          The config structure to populate.
 *
 * \returns 0 on success and non-zero on failure.
 */
int config_read_proc(struct bootstrap_config* bconf, agent_config_t* conf)
{
    int retval = 1;
    int clientsock = -1, serversock = -1;
    int procid = 0;
    uid_t uid;
    gid_t gid;

    /* verify that this process is running as root. */
    if (0 != geteuid())
    {
        fprintf(stderr, "agentd must be run as root.\n");
        retval = 1;
        goto done;
    }

    /* create a socketpair for communication. */
    retval = ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &clientsock, &serversock);
    if (0 != retval)
    {
        perror("ipc_socketpair");
        goto done;
    }

    /* fork the process into parent and child. */
    procid = fork();
    if (0 > procid)
    {
        perror("fork");
        retval = procid;
        goto done;
    }

    /* child */
    if (0 == procid)
    {
        /* close parent's end of the socket pair. */
        close(clientsock);
        clientsock = -1;

        /* get the user and group IDs for nobody. */
        retval = privsep_lookup_usergroup("nobody", "nogroup", &uid, &gid);
        if (0 != retval)
        {
            perror("privsep_lookup_usergroup");
            goto done;
        }

        /* change into the prefix directory. */
        retval = privsep_chroot(bconf->prefix_dir);
        if (0 != retval)
        {
            perror("privsep_chroot");
            goto done;
        }

        /* set the user ID and group ID. */
        retval = privsep_drop_privileges(uid, gid);
        if (0 != retval)
        {
            perror("privsep_drop_privileges");
            goto done;
        }

        /* open config file. */
        int config_fd = open(bconf->config_file, O_RDONLY);
        if (0 > config_fd)
        {
            perror("config open");
            retval = config_fd;
            goto done;
        }

        /* close standard file descriptors and reset config_fd. */
        retval =
            privsep_setfds(
                config_fd, /* ==> */ AGENTD_FD_CONFIG_IN,
                serversock, /* ==> */ AGENTD_FD_CONFIG_OUT,
                -1);
        if (0 != retval)
        {
            perror("privsep_setfds");
            goto done;
        }

        /* spawn the child process (this does not return if successful. */
        retval = privsep_exec_private("readconfig");
        if (0 != retval)
        {
            perror("privsep_exec_private");
            goto done;
        }

        printf("Should never get here.\n");
        retval = 1;
        goto done;
    }
    /* parent */
    else
    {
        int pidstatus;
        close(serversock);
        serversock = -1;

        /* read the config data from the client socket. */
        if (0 != config_read_block(clientsock, conf))
        {
            retval = 1;
            goto done;
        }

        /* wait on the child process to complete. */
        waitpid(procid, &pidstatus, 0);

        /* Use the return value of the child process as our return value. */
        if (WIFEXITED(pidstatus))
        {
            retval = WEXITSTATUS(pidstatus);
        }
        else
        {
            retval = 1;
        }

        /* provide defaults for any config value not set. */
        if (0 == retval && 0 != config_set_defaults(conf, bconf))
        {
            retval = 2;
            goto cleanup_config;
        }

        goto done;
    }

cleanup_config:
    dispose((disposable_t*)conf);

done:
    /* clean up clientsock. */
    if (0 <= clientsock)
        close(clientsock);

    /* clean up serversock. */
    if (0 <= serversock)
        close(serversock);

    return retval;
}
