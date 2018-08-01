/**
 * \file config/config_read_block.c
 *
 * \brief Read a config structure from the given stream.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <agentd/ipc.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>

/* forward decls */
static int config_read_logdir(int s, agent_config_t* conf);
static int config_read_loglevel(int s, agent_config_t* conf);
static int config_read_secret(int s, agent_config_t* conf);
static int config_read_rootblock(int s, agent_config_t* conf);
static int config_read_datastore(int s, agent_config_t* conf);
static int config_read_chroot(int s, agent_config_t* conf);
static int config_read_usergroup(int s, agent_config_t* conf);
static int config_read_listen_addr(int s, agent_config_t* conf);

/**
 * \brief Initialize and read an agent config structure from a blocking stream.
 *
 * On success, a config structure is initialized with data from the blocking
 * stream.  This is owned by the caller and must be disposed by calling \ref
 * dispose() when no longer needed.
 *
 * \param s             The socket descriptor to read.
 * \param conf          The config structure to read.
 *
 * \returns 0 on success and non-zero on failure.
 */
int config_read_block(int s, agent_config_t* conf)
{
    uint8_t type;

    /* initialize this config structure. */
    memset(conf, 0, sizeof(agent_config_t));
    conf->hdr.dispose = &config_dispose;

    /* get the BOM to start this stream. */
    if (0 != ipc_read_uint8_block(s, &type))
        return 1;

    /* verify that we have the beginning of this stream. */
    if (CONFIG_STREAM_TYPE_BOM != type)
        return 2;

    /* read all config data. */
    while (0 == ipc_read_uint8_block(s, &type))
    {
        /* handle different config fields. */
        switch (type)
        {
            /* end of stream.  Success. */
            case CONFIG_STREAM_TYPE_EOM:
                return 0;

            /* logdir */
            case CONFIG_STREAM_TYPE_LOGDIR:
                /* attempt to read the logdir from the stream. */
                if (0 != config_read_logdir(s, conf))
                    return 3;
                break;

            /* loglevel */
            case CONFIG_STREAM_TYPE_LOGLEVEL:
                /* attempt to read the loglevel from the stream. */
                if (0 != config_read_loglevel(s, conf))
                    return 4;
                break;

            /* secret */
            case CONFIG_STREAM_TYPE_SECRET:
                /* attempt to read the secret from the stream. */
                if (0 != config_read_secret(s, conf))
                    return 5;
                break;

            /* rootblock */
            case CONFIG_STREAM_TYPE_ROOTBLOCK:
                /* attempt to read the rootblock from the stream. */
                if (0 != config_read_rootblock(s, conf))
                    return 6;
                break;

            /* datastore */
            case CONFIG_STREAM_TYPE_DATASTORE:
                /* attempt to read the datastore from the stream. */
                if (0 != config_read_datastore(s, conf))
                    return 7;
                break;

            /* listen address */
            case CONFIG_STREAM_TYPE_LISTEN_ADDR:
                /* attempt to read a listen address. */
                if (0 != config_read_listen_addr(s, conf))
                    return 8;
                break;

            /* chroot */
            case CONFIG_STREAM_TYPE_CHROOT:
                /* attempt to read the chroot from the stream. */
                if (0 != config_read_chroot(s, conf))
                    return 9;
                break;

            /* usergroup */
            case CONFIG_STREAM_TYPE_USERGROUP:
                /* attempt to read the usergroup from the stream. */
                if (0 != config_read_usergroup(s, conf))
                    return 10;
                break;

            /* unknown data */
            default:
                /* return error. */
                return 11;
        }
    }

    /* if we make it here, something has gone wrong. */
    return 12;
}

/**
 * \brief Read the logdir from the config stream.
 *
 * \param s             The socket from which the logdir is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_read_logdir(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->logdir)
        return 1;

    /* attempt to read the logdir. */
    if (0 != ipc_read_string_block(s, (char**)&conf->logdir))
        return 2;

    /* success */
    return 0;
}

/**
 * \brief Read the loglevel from the config stream.
 *
 * \param s             The socket from which the loglevel is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_read_loglevel(int s, agent_config_t* conf)
{
    /* it's an error to set the loglevel more than once. */
    if (conf->loglevel_set)
        return 1;

    /* attempt to read the loglevel. */
    if (0 != ipc_read_int64_block(s, &conf->loglevel))
        return 2;

    /* loglevel must be between 0 and 9. */
    if (conf->loglevel < 0 || conf->loglevel > 9)
        return 3;

    /* loglevel has been set. */
    conf->loglevel_set = true;

    /* success. */
    return 0;
}

/**
 * \brief Read the secret from the config stream.
 *
 * \param s             The socket from which the secret is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_read_secret(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->secret)
        return 1;

    /* attempt to read the secret. */
    if (0 != ipc_read_string_block(s, (char**)&conf->secret))
        return 2;

    /* success */
    return 0;
}

/**
 * \brief Read the rootblock from the config stream.
 *
 * \param s             The socket from which the rootblock is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_read_rootblock(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->rootblock)
        return 1;

    /* attempt to read the rootblock. */
    if (0 != ipc_read_string_block(s, (char**)&conf->rootblock))
        return 2;

    /* success */
    return 0;
}

/**
 * \brief Read the datastore from the config stream.
 *
 * \param s             The socket from which the datastore is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_read_datastore(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->datastore)
        return 1;

    /* attempt to read the datastore. */
    if (0 != ipc_read_string_block(s, (char**)&conf->datastore))
        return 2;

    /* success */
    return 0;
}

/**
 * \brief Read the chroot from the config stream.
 *
 * \param s             The socket from which the chroot is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_read_chroot(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->chroot)
        return 1;

    /* attempt to read the chroot. */
    if (0 != ipc_read_string_block(s, (char**)&conf->chroot))
        return 2;

    /* success */
    return 0;
}

/**
 * \brief Read the user/group from the config stream.
 *
 * \param s             The socket from which the user/group is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_read_usergroup(int s, agent_config_t* conf)
{
    /* it's an error to provide this value more than once. */
    if (NULL != conf->usergroup)
        return 1;

    /* allocate a usergroup structure. */
    config_user_group_t* usergroup =
        (config_user_group_t*)malloc(sizeof(config_user_group_t));
    if (NULL == usergroup)
        return 2;

    /* clear this structure. */
    memset(usergroup, 0, sizeof(config_user_group_t));

    /* attempt to read the user. */
    if (0 != ipc_read_string_block(s, (char**)&usergroup->user))
    {
        free(usergroup);
        return 3;
    }

    /* attempt to read the group. */
    if (0 != ipc_read_string_block(s, (char**)&usergroup->group))
    {
        free((char*)usergroup->user);
        free(usergroup);
        return 4;
    }

    /* set the usergroup. */
    conf->usergroup = usergroup;

    /* success */
    return 0;
}

/**
 * \brief Read a listen address from the config stream.
 *
 * \param s             The socket from which the listen address is read.
 * \param conf          The config structure instance to write this value.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_read_listen_addr(int s, agent_config_t* conf)
{
    int retval = 1;
    char* paddr = NULL;
    config_listen_address_t* ptr = NULL;

    /* allocate a listen_addr structure. */
    ptr = (config_listen_address_t*)malloc(sizeof(config_listen_address_t));
    if (NULL == ptr)
    {
        retval = 1;
        goto done;
    }

    /* clear this structure. */
    memset(ptr, 0, sizeof(config_listen_address_t));

    /* attempt to read the address. */
    if (0 != ipc_read_string_block(s, &paddr))
    {
        retval = 2;
        goto cleanup;
    }

    /* allocate an addr structure. */
    ptr->addr = (struct in_addr*)malloc(sizeof(struct in_addr));
    if (NULL == ptr->addr)
    {
        retval = 3;
        goto cleanup;
    }

    /* convert to a network address. */
    if (1 != inet_pton(AF_INET, paddr, ptr->addr))
    {
        retval = 4;
        goto cleanup;
        return 4;
    }

    /* clean up. */
    free(paddr);
    paddr = NULL;

    /* attempt to write the listen port. */
    uint64_t port = 0U;
    if (0 != ipc_read_uint64_block(s, &port))
    {
        retval = 5;
        goto cleanup;
    }

    /* set port. */
    ptr->port = port;

    /* append the address. */
    ptr->hdr.next = (config_list_node_t*)conf->listen_head;
    conf->listen_head = ptr;
    ptr = NULL;

    /* success */
    retval = 0;

cleanup:

    /* clean up this failed node. */
    if (NULL != ptr)
    {
        /* clean up addr */
        if (NULL != ptr->addr)
            free(ptr->addr);

        free(ptr);
    }

    /* clean up presentation address. */
    if (NULL != paddr)
        free(paddr);

done:
    return retval;
}
