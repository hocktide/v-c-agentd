/**
 * \file config/config_read_proc.c
 *
 * \brief Spawn an unprivileged process to read and verify the config file.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <agentd/config.h>
#include <agentd/fds.h>
#include <agentd/ipc.h>
#include <agentd/privsep.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <errno.h>
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
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_PROC_RUNSECURE_ROOT_USER_REQUIRED if spawning this
 *        process failed because the user is not root and runsecure is true.
 *      - AGENTD_ERROR_CONFIG_IPC_SOCKETPAIR_FAILURE if creating a socketpair
 *        for the dataservice process failed.
 *      - AGENTD_ERROR_CONFIG_FORK_FAILURE if forking the private process
 *        failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_LOOKUP_USERGROUP_FAILURE if there was a
 *        failure looking up the configured user and group for the dataservice
 *        process.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_CHROOT_FAILURE if chrooting failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_DROP_PRIVILEGES_FAILURE if dropping
 *        privileges failed.
 *      - AGENTD_ERROR_CONFIG_OPEN_CONFIG_FILE_FAILURE if opening the config
 *        file failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_SETFDS_FAILURE if setting file descriptors
 *        failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_EXEC_PRIVATE_FAILURE if executing the
 *        private command failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS if the process
 *        survived execution (weird!).      
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if reading data from the
 *        config stream failed.
 *      - AGENTD_ERROR_CONFIG_PROC_EXIT_FAILURE if the config proc did not
 *        properly exit.
 *      - AGENTD_ERROR_CONFIG_DEFAULTS_SET_FAILURE if setting the config
 *        defaults failed.
 */
int config_read_proc(
    const struct bootstrap_config* bconf, agent_config_t* conf)
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
        retval = AGENTD_ERROR_CONFIG_PROC_RUNSECURE_ROOT_USER_REQUIRED;
        goto done;
    }

    /* create a socketpair for communication. */
    retval = ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &clientsock, &serversock);
    if (0 != retval)
    {
        perror("ipc_socketpair");
        retval = AGENTD_ERROR_CONFIG_IPC_SOCKETPAIR_FAILURE;
        goto done;
    }

    /* fork the process into parent and child. */
    procid = fork();
    if (procid < 0)
    {
        perror("fork");
        retval = AGENTD_ERROR_CONFIG_FORK_FAILURE;
        goto done;
    }

    /* child */
    if (0 == procid)
    {
        /* close parent's end of the socket pair. */
        close(clientsock);
        clientsock = -1;

        /* get the user and group IDs for nobody:nogroup. */
        retval = privsep_lookup_usergroup("nobody", "nogroup", &uid, &gid);
        if (AGENTD_ERROR_GENERAL_GETGRNAM_FAILURE == retval)
        {
            /* attempt nobody:nobody if nobody:nogroup fails. */
            retval = privsep_lookup_usergroup("nobody", "nobody", &uid, &gid);
        }

        /* if group lookup failed, handle the error. */
        if (0 != retval)
        {
            perror("privsep_lookup_usergroup");
            retval = AGENTD_ERROR_CONFIG_PRIVSEP_LOOKUP_USERGROUP_FAILURE;
            goto done;
        }

        /* change into the prefix directory. */
        retval = privsep_chroot(bconf->prefix_dir);
        if (0 != retval)
        {
            perror("privsep_chroot");
            retval = AGENTD_ERROR_CONFIG_PRIVSEP_CHROOT_FAILURE;
            goto done;
        }

        /* set the user ID and group ID. */
        retval = privsep_drop_privileges(uid, gid);
        if (0 != retval)
        {
            perror("privsep_drop_privileges");
            retval = AGENTD_ERROR_CONFIG_PRIVSEP_DROP_PRIVILEGES_FAILURE;
            goto done;
        }

        /* if the config file is not set, default to /etc/agentd.conf */
        const char* config_file = bconf->config_file;
        if (NULL == config_file)
        {
            config_file = "/etc/agentd.conf";
        }

        /* open config file. */
        int config_fd = open(config_file, O_RDONLY);
        if (0 > config_fd)
        {
            perror("config open");
            retval = AGENTD_ERROR_CONFIG_OPEN_CONFIG_FILE_FAILURE;
            goto done;
        }

        /* move the fds out of the way. */
        if (AGENTD_STATUS_SUCCESS !=
            privsep_protect_descriptors(&config_fd, &serversock, NULL))
        {
            retval = AGENTD_ERROR_CONFIG_PRIVSEP_SETFDS_FAILURE;
            goto done;
        }

        /* close standard file descriptors */
        retval = privsep_close_standard_fds();
        if (0 != retval)
        {
            perror("privsep_close_standard_fds");
            retval = AGENTD_ERROR_CONFIG_PRIVSEP_SETFDS_FAILURE;
            goto done;
        }

        /* reset config_fd. */
        retval =
            privsep_setfds(
                config_fd, /* ==> */ AGENTD_FD_CONFIG_IN,
                serversock, /* ==> */ AGENTD_FD_CONFIG_OUT,
                -1);
        if (0 != retval)
        {
            perror("privsep_setfds");
            retval = AGENTD_ERROR_CONFIG_PRIVSEP_SETFDS_FAILURE;
            goto done;
        }

        /* spawn the child process (this does not return if successful. */
        retval = privsep_exec_private(bconf, "readconfig");
        if (0 != retval)
        {
            perror("privsep_exec_private");
            retval = AGENTD_ERROR_CONFIG_PRIVSEP_EXEC_PRIVATE_FAILURE;
            goto done;
        }

        printf("Should never get here.\n");
        retval = AGENTD_ERROR_CONFIG_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS;
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
            retval = AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE;
            goto done;
        }

        /* wait on the child process to complete. */
        waitpid(procid, &pidstatus, 0);

        /* Use the return value of the child process as our return value. */
        if (WIFEXITED(pidstatus))
        {
            retval = WEXITSTATUS(pidstatus);
            if (0 != retval)
            {
                retval = AGENTD_ERROR_CONFIG_PROC_EXIT_FAILURE;
            }
        }
        else
        {
            retval = AGENTD_ERROR_CONFIG_PROC_EXIT_FAILURE;
        }

        /* provide defaults for any config value not set. */
        if (0 != config_set_defaults(conf, bconf))
        {
            retval = AGENTD_ERROR_CONFIG_DEFAULTS_SET_FAILURE;
            goto cleanup_config;
        }

        goto done;
    }

cleanup_config:
    dispose((disposable_t*)conf);

done:
    /* clean up clientsock. */
    if (clientsock >= 0)
        close(clientsock);

    /* clean up serversock. */
    if (serversock >= 0)
        close(serversock);

    return retval;
}
