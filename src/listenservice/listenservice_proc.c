/**
 * \file listenservice/listenservice_proc.c
 *
 * \brief Spawn the listen service process.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <agentd/config.h>
#include <agentd/fds.h>
#include <agentd/ipc.h>
#include <agentd/listenservice.h>
#include <agentd/privsep.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vpr/parameters.h>

/* forward decls */
int listenservice_proc_open_listen_sockets(
    const bootstrap_config_t* bconf, const agent_config_t* conf);

/**
 * \brief Spawn an unauthorized listen service process using the provided
 * config structure and logger socket.
 *
 * On success, this method sets the file descriptor pointer to the file
 * descriptor for the listen service socket.  This can be used by the caller to
 * send requests to the listen service and to receive responses from this
 * service. Also, the pointer to the pid for this process is set.  This can be
 * used to signal and wait when this process should be terminated.
 *
 * \param bconf         The bootstrap configuration for this service.
 * \param conf          The configuration for this service.
 * \param acceptsock    Socket used to pass accepted sockets.
 * \param logsock       Socket used to communicate with the logger.
 * \param listenpid     Pointer to the listen service pid, to be updated on
 *                      the successful completion of this function.
 * \param runsecure     Set to false if we are not being run in secure mode.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_LISTENSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED if
 *        spawning this process failed because the user is not root and
 *        runsecure is true.
 *      - AGENTD_ERROR_LISTENSERVICE_FORK_FAILURE if forking the private
 *        process failed.
 *      - AGENTD_ERROR_LISTENSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE if there
 *        was a failure looking up the configured user and group for the
 *        listen service process.
 *      - AGENTD_ERROR_LISTENSERVICE_PRIVSEP_CHROOT_FAILURE if chrooting
 *        failed.
 *      - AGENTD_ERROR_LISTENSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE if
 *        dropping privileges failed.
 *      - AGENTD_ERROR_LISTENSERVICE_PRIVSEP_SETFDS_FAILURE if setting file
 *        descriptors failed.
 *      - AGENTD_ERROR_LISTENSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE if executing
 *        the private command failed.
 *      - AGENTD_ERROR_LISTENSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS if the
 *        process survived execution (weird!).      
 */
int listenservice_proc(
    const bootstrap_config_t* bconf, const agent_config_t* conf, int acceptsock,
    int logsock, pid_t* listenpid, bool runsecure)
{
    int retval = 1;
    uid_t uid;
    gid_t gid;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != bconf);
    MODEL_ASSERT(NULL != conf);
    MODEL_ASSERT(NULL != logsock);
    MODEL_ASSERT(NULL != listenpid);

    /* verify that this process is running as root. */
    if (runsecure && 0 != geteuid())
    {
        fprintf(stderr, "agentd must be run as root.\n");
        retval = AGENTD_ERROR_LISTENSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED;
        goto done;
    }

    /* fork the process into parent and child. */
    *listenpid = fork();
    if (*listenpid < 0)
    {
        perror("fork");
        retval = AGENTD_ERROR_LISTENSERVICE_FORK_FAILURE;
        goto done;
    }

    /* child */
    if (0 == *listenpid)
    {
        /* move the fds out of the way. */
        if (AGENTD_STATUS_SUCCESS !=
            privsep_protect_descriptors(&logsock, &acceptsock, NULL))
        {
            retval = AGENTD_ERROR_LISTENSERVICE_PRIVSEP_SETFDS_FAILURE;
            goto done;
        }

        /* close standard file descriptors */
        retval = privsep_close_standard_fds();
        if (0 != retval)
        {
            perror("privsep_close_standard_fds");
            retval = AGENTD_ERROR_LISTENSERVICE_PRIVSEP_SETFDS_FAILURE;
            goto done;
        }

        /* close standard file descriptors and set fds. */
        retval =
            privsep_setfds(
                logsock, /* ==> */ AGENTD_FD_LISTENSERVICE_LOG,
                acceptsock, /* ==> */ AGENTD_FD_LISTENSERVICE_ACCEPT,
                -1);
        if (0 != retval)
        {
            perror("privsep_setfds");
            retval = AGENTD_ERROR_LISTENSERVICE_PRIVSEP_SETFDS_FAILURE;
            goto done;
        }

        /* open listen sockets. */
        retval = listenservice_proc_open_listen_sockets(bconf, conf);
        if (0 != retval)
        {
            retval =
                AGENTD_ERROR_LISTENSERVICE_LISTENSOCKET_OPEN_FAILURE;
            goto done;
        }

        /* do secure operations if requested. */
        if (runsecure)
        {
            /* get the user and group IDs. */
            retval =
                privsep_lookup_usergroup(
                    conf->usergroup->user, conf->usergroup->group, &uid, &gid);
            if (0 != retval)
            {
                perror("privsep_lookup_usergroup");
                retval =
                    AGENTD_ERROR_LISTENSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE;
                goto done;
            }

            /* change into the prefix directory. */
            retval = privsep_chroot(bconf->prefix_dir);
            if (0 != retval)
            {
                perror("privsep_chroot");
                retval = AGENTD_ERROR_LISTENSERVICE_PRIVSEP_CHROOT_FAILURE;
                goto done;
            }

            /* set the user ID and group ID. */
            retval = privsep_drop_privileges(uid, gid);
            if (0 != retval)
            {
                perror("privsep_drop_privileges");
                retval =
                    AGENTD_ERROR_LISTENSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE;
                goto done;
            }

            /* spawn the child process (this does not return if successful). */
            if (runsecure)
            {
                retval = privsep_exec_private(bconf, "listenservice");
            }
        }
        else
        {
            /* if running in non-secure mode, then we expect the caller to have
             * already set the path and library path accordingly. */
            retval = execlp(
                "agentd", "agentd", "-P", "listenservice",
                NULL);
        }

        /* check the exec status. */
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            perror("privsep_exec_private");
            retval = AGENTD_ERROR_LISTENSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE;
            goto done;
        }

        /* we'll never get here. */
        retval = AGENTD_ERROR_LISTENSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS;
        goto done;
    }
    /* parent */
    else
    {
        /* success. */
        retval = AGENTD_STATUS_SUCCESS;
        goto done;
    }

done:

    return retval;
}

/**
 * \brief Open the listen sockets for agentd.
 *
 * \param bconf         The bootstrap config for this process.
 * \param conf          The config for this process.
 *
 * \returns AGENTD_STATUS_SUCCESS on success and a non-zero status on failure.
 */
int listenservice_proc_open_listen_sockets(
    const bootstrap_config_t* UNUSED(bconf), const agent_config_t* conf)
{
    int retval = 0;
    int setsock = AGENTD_FD_LISTENSERVICE_SOCK_START;
    const config_listen_address_t* listen_head = conf->listen_head;

    while (NULL != listen_head)
    {
        /* create a listen socket. */
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            return AGENTD_ERROR_LISTENSERVICE_LISTENSOCKET_OPEN_FAILURE;
        }

        /* create a sockaddr_in entry. */
        struct sockaddr_in saddr;
        memset(&saddr, 0, sizeof(saddr));
        saddr.sin_family = AF_INET;
        saddr.sin_port = htons(listen_head->port);
        memcpy(&saddr.sin_addr, listen_head->addr, sizeof(saddr.sin_addr));

        /* bind this address to the address in listen_head. */
        retval = bind(sock, (const struct sockaddr*)&saddr, sizeof(saddr));
        if (retval < 0)
        {
            return AGENTD_ERROR_LISTENSERVICE_LISTENSOCKET_OPEN_FAILURE;
        }

        /* start listening on this socket. */
        retval = listen(sock, 16);
        if (retval < 0)
        {
            return AGENTD_ERROR_LISTENSERVICE_LISTENSOCKET_OPEN_FAILURE;
        }

        /* if the socket is not the next socket, dup2 it over. */
        if (sock != setsock)
        {
            retval = dup2(sock, setsock);
            if (retval < 0)
            {
                return AGENTD_ERROR_LISTENSERVICE_LISTENSOCKET_OPEN_FAILURE;
            }

            /* close the old socket. */
            close(sock);
        }

        /* skip to the next listen address.*/
        listen_head = (const config_listen_address_t*)listen_head->hdr.next;

        /* skip to the next set socket. */
        ++setsock;
    }

    return AGENTD_STATUS_SUCCESS;
}
