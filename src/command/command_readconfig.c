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
    agent_config_t conf;

    /* read the config, spawning a process to do so. */
    retval = config_read_proc(bconf, &conf);
    if (0 != retval)
    {
        return retval;
    }

    /* output the config data. */
    if (NULL != conf.logdir)
        printf("Log directory: %s\n", conf.logdir);
    if (conf.loglevel_set)
        printf("Log level: %d\n", (int)conf.loglevel);
    if (conf.block_max_seconds_set)
        printf("Consensus max seconds: %d\n", (int)conf.block_max_seconds);
    if (conf.block_max_transactions_set)
        printf("Consensus max transactions: %d\n",
            (int)conf.block_max_transactions);
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

    /* clean up config. */
    dispose((disposable_t*)&conf);

    /* success. */
    return 0;
}
