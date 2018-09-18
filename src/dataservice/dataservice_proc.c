/**
 * \file dataservice/dataservice_proc.c
 *
 * \brief Spawn the dataservice process.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <agentd/config.h>
#include <agentd/dataservice.h>
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
 * \brief Spawn a data service process using the provided config structure and
 * logger socket.
 *
 * On success, this method sets the file descriptor pointer to the file
 * descriptor for the data service socket.  This can be used by the caller to
 * send requests to the data service and to receive responses from this service.
 * Also, the pointer to the pid for this process is set.  This can be used to
 * signal and wait when this process should be terminated.
 *
 * \param bconf         The bootstrap configuration for this service.
 * \param conf          The configuration for this service.
 * \param logsock       Socket used to communicate with the logger.
 * \param datasock      Pointer to the data service socket, to be updated on
 *                      successful completion of this function.
 * \param datapid       Pointer to the data service pid, to be updated on the
 *                      successful completion of this function.
 * \param runsecure     Set to false if we are not being run in secure mode.
 *
 * \returns 0 on success and non-zero on failure.
 */
int dataservice_proc(
    const bootstrap_config_t* bconf, const agent_config_t* conf, int logsock,
    int* datasock, pid_t* datapid, bool runsecure)
{
    int retval = 1;
    int serversock = -1;
    bool keep_datasock = false;
    uid_t uid;
    gid_t gid;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != bconf);
    MODEL_ASSERT(NULL != conf);
    MODEL_ASSERT(NULL != datasock);
    MODEL_ASSERT(NULL != datapid);

    /* sentinel value for data socket. */
    *datasock = -1;

    /* verify that this process is running as root. */
    if (runsecure && 0 != geteuid())
    {
        fprintf(stderr, "agentd must be run as root.\n");
        retval = 1;
        goto done;
    }

    /* create a socketpair for communication. */
    retval = ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, datasock, &serversock);
    if (0 != retval)
    {
        perror("ipc_socketpair");
        goto done;
    }

    /* fork the process into parent and child. */
    *datapid = fork();
    if (*datapid < 0)
    {
        perror("fork");
        retval = *datapid;
        goto done;
    }

    /* child */
    if (0 == *datapid)
    {
        /* close the parent's end of the socket pair. */
        close(*datasock);
        *datasock = -1;

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
        }

        /* close standard file descriptors and set fds. */
        retval =
            privsep_setfds(
                serversock, /* ==> */ AGENTD_FD_DATASERVICE_SOCK,
                logsock, /* ==> */ AGENTD_FD_DATASERVICE_LOG,
                -1);
        if (0 != retval)
        {
            perror("privsep_setfds");
            goto done;
        }

        /* spawn the child process (this does not return if successful). */
        if (runsecure)
        {
            retval = privsep_exec_private("dataservice");
        }
        else
        {
            /* if running in non-secure mode, then we expect the caller to have
             * already set the path and library path accordingly. */
            retval = execlp("agentd", "agentd", "-P", "dataservice", NULL);
        }

        /* check the exec status. */
        if (0 != retval)
        {
            perror("privsep_exec_private");
            goto done;
        }

        /* we'll never get here. */
        retval = 1;
        goto done;
    }
    /* parent */
    else
    {
        /* close the child's end of the socket pair. */
        close(serversock);
        serversock = -1;

        /* let the cleanup logic know that we want to preserve datasock. */
        keep_datasock = true;

        /* success. */
        retval = 0;
        goto done;
    }

done:
    /* clean up server socket. */
    if (serversock >= 0)
    {
        close(serversock);
    }

    /* clean up data socket. */
    if (!keep_datasock && *datasock >= 0)
    {
        close(*datasock);
    }

    return retval;
}
