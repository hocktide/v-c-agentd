/**
 * \file command/command_readconfig.c
 *
 * \brief Read and verify the config file, writing human readable settings to
 * standard output.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/config.h>
#include <agentd/fds.h>
#include <agentd/ipc.h>
#include <agentd/privsep.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * \brief Read and verify the config file, writing human readable settings to
 * standard output.
 *
 * \param bconf         The bootstrap configuration for this command.
 *
 * \returns 0 on success and non-zero on failure.
 */
int command_readconfig(struct bootstrap_config* bconf)
{
    int retval = 1;
    int clientsock, serversock;
    int procid = 0;
    uid_t uid;
    gid_t gid;

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
        agent_config_t conf;
        close(serversock);

        /* read the config data from the client socket. */
        if (0 != config_read_block(clientsock, &conf))
        {
            retval = 1;
            goto done;
        }

        /* output the config data. */
        if (NULL != conf.logdir)
            printf("Log directory: %s\n", conf.logdir);
        if (conf.loglevel_set)
            printf("Log level: %lld\n", conf.loglevel);
        if (NULL != conf.secret)
            printf("Secret file: %s\n", conf.secret);
        if (NULL != conf.rootblock)
            printf("Root block file: %s\n", conf.rootblock);
        if (NULL != conf.datastore)
            printf("Datastore Directory: %s\n", conf.datastore);
        if (NULL != conf.chroot)
            printf("Chroot Directory: %s\n", conf.chroot);
        if (NULL != conf.usergroup)
            printf("User:Group: %s:%s\n",
                conf.usergroup->user,
                conf.usergroup->group);

        /* output listen addresses. */
        config_listen_address_t* ptr = conf.listen_head;
        while (NULL != ptr)
        {
            char txt[1024];

            printf("Listen Address: %s:%d\n",
                inet_ntop(AF_INET, ptr->addr, txt, sizeof(txt)), ptr->port);

            ptr = (config_listen_address_t*)ptr->hdr.next;
        }

        /* wait on the child process to complete. */
        waitpid(procid, &pidstatus, 0);

        /* Use the return value of the child process as our return value. */
        if (WIFEXITED(pidstatus))
            retval = WEXITSTATUS(pidstatus);
        else
            retval = 1;

        goto done;
    }

done:
    return retval;
}
