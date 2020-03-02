/**
 * \file randomservice/random_service_proc.c
 *
 * \brief Spawn the random service process.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <agentd/config.h>
#include <agentd/fds.h>
#include <agentd/ipc.h>
#include <agentd/privsep.h>
#include <agentd/protocolservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Spawn a random service process using the provided config structure and
 * logger socket.
 *
 * On success, this method sets the file descriptor pointer to the file
 * descriptors for the random service socket.  This can be used by the caller to
 * send requests to the random service and to receive responses from this
 * service.  Also, the pointer to the pid for this process is set.  This can be
 * used to signal and wait when this process should be terminated.
 *
 * \param bconf         The bootstrap configuration for this service.
 * \param conf          The configuration for this service.
 * \param logsock       Socket used to communicate with the logger.
 * \param protosock     Socket used to communicate with the protocol service.
 * \param randompid     Pointer to the random service pid, to be updated on
 *                      the successful completion of this function.
 * \param runsecure     Set to false if we are not being run in secure mode.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_RANDOMSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED if
 *        spawning this process failed because the user is not root and
 *        runsecure is true.
 *      - AGENTD_ERROR_RANDOMSERVICE_IPC_SOCKETPAIR_FAILURE if creating a
 *        socketpair for the random service process failed.
 *      - AGENTD_ERROR_RANDOMSERVICE_FORK_FAILURE if forking the private
 *        process failed.
 *      - AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE if there
 *        was a failure looking up the configured user and group for the
 *        random service process.
 *      - AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_CHROOT_FAILURE if chrooting
 *        failed.
 *      - AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE if
 *        dropping privileges failed.
 *      - AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_SETFDS_FAILURE if setting file
 *        descriptors failed.
 *      - AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE if executing
 *        the private command failed.
 *      - AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS if the
 *        process survived execution (weird!).      
 */
int randomservice_proc(
    const bootstrap_config_t* bconf, const agent_config_t* conf, int logsock,
    int* protosock, pid_t* randompid, bool runsecure)
{
    int retval = 1;
    uid_t uid;
    gid_t gid;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != bconf);
    MODEL_ASSERT(NULL != conf);
    MODEL_ASSERT(NULL != randompid);

    /* verify that this process is running as root. */
    if (runsecure && 0 != geteuid())
    {
        fprintf(stderr, "agentd must be run as root.\n");
        retval = AGENTD_ERROR_RANDOMSERVICE_PROC_RUNSECURE_ROOT_USER_REQUIRED;
        goto done;
    }

    /* create a socketpair for protocol service. */
    int random_proto = -1;
    retval = ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, protosock, &random_proto);
    if (0 != retval)
    {
        perror("ipc_socketpair");
        retval = AGENTD_ERROR_RANDOMSERVICE_IPC_SOCKETPAIR_FAILURE;
        goto done;
    }

    /* fork the process into parent and child. */
    *randompid = fork();
    if (*randompid < 0)
    {
        perror("fork");
        retval = AGENTD_ERROR_RANDOMSERVICE_FORK_FAILURE;
        goto done;
    }

    /* child */
    if (0 == *randompid)
    {
        /* open the random device socket. */
        int random = open("/dev/urandom", O_RDONLY);
        if (random < 0)
        {
            retval = AGENTD_ERROR_RANDOMSERVICE_RANDOM_DEVICE_OPEN_FAILURE;
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
                    AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_LOOKUP_USERGROUP_FAILURE;
                goto done;
            }

            /* change into the prefix directory. */
            retval = privsep_chroot(bconf->prefix_dir);
            if (0 != retval)
            {
                perror("privsep_chroot");
                retval = AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_CHROOT_FAILURE;
                goto done;
            }

            /* set the user ID and group ID. */
            retval = privsep_drop_privileges(uid, gid);
            if (0 != retval)
            {
                perror("privsep_drop_privileges");
                retval =
                    AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_DROP_PRIVILEGES_FAILURE;
                goto done;
            }
        }

        /* move the fds out of the way. */
        if (AGENTD_STATUS_SUCCESS !=
            privsep_protect_descriptors(
                &random, &random_proto, &logsock, NULL))
        {
            retval = AGENTD_ERROR_CONFIG_PRIVSEP_SETFDS_FAILURE;
            goto done;
        }

        /* close standard file descriptors */
        retval = privsep_close_standard_fds();
        if (0 != retval)
        {
            perror("privsep_close_standard_fds");
            retval = AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_SETFDS_FAILURE;
            goto done;
        }

        /* close standard file descriptors and set fds. */
        retval =
            privsep_setfds(
                random, /* ==> */ AGENTD_FD_RANDOM_SERVICE_RANDOM_DEVICE,
                random_proto, /* ==> */ AGENTD_FD_RANDOM_SERVICE_PROTOCOL_SERVICE,
                logsock, /* ==> */ AGENTD_FD_RANDOM_SERVICE_LOG_SOCKET,
                -1);
        if (0 != retval)
        {
            perror("privsep_setfds");
            retval = AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_SETFDS_FAILURE;
            goto done;
        }

        /* spawn the child process (this does not return if successful). */
        if (runsecure)
        {
            retval = privsep_exec_private(bconf, "random");
        }
        else
        {
            /* if running in non-secure mode, then we expect the caller to have
             * already set the path and library path accordingly. */
            retval = execlp("agentd", "agentd", "-P", "random", NULL);
        }

        /* check the exec status. */
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            perror("privsep_exec_private");
            retval = AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_EXEC_PRIVATE_FAILURE;
            goto done;
        }

        /* we'll never get here. */
        retval = AGENTD_ERROR_RANDOMSERVICE_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS;
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
