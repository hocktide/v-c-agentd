/**
 * \file config/config_write_block.c
 *
 * \brief Write a config structure to the given stream.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <agentd/ipc.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <unistd.h>

/* forward decls */
static int config_write_logdir(int s, agent_config_t* conf);
static int config_write_loglevel(int s, agent_config_t* conf);
static int config_write_secret(int s, agent_config_t* conf);
static int config_write_rootblock(int s, agent_config_t* conf);
static int config_write_datastore(int s, agent_config_t* conf);
static int config_write_listen_addr(int s, agent_config_t* conf);
static int config_write_chroot(int s, agent_config_t* conf);
static int config_write_usergroup(int s, agent_config_t* conf);

/**
 * \brief Write a config structure to a blocking stream.
 *
 * \param s             The socket descriptor to write.
 * \param conf          The config structure to write.
 *
 * \returns 0 on success and non-zero on failure.
 */
int config_write_block(int s, agent_config_t* conf)
{
    uint8_t type = 0U;

    /* begin config data. */
    type = CONFIG_STREAM_TYPE_BOM;
    if (0 != ipc_write_uint8_block(s, type))
        return 1;

    /* logdir */
    if (0 != config_write_logdir(s, conf))
        return 2;

    /* loglevel */
    if (0 != config_write_loglevel(s, conf))
        return 3;

    /* secret */
    if (0 != config_write_secret(s, conf))
        return 4;

    /* rootblock */
    if (0 != config_write_rootblock(s, conf))
        return 5;

    /* datastore */
    if (0 != config_write_datastore(s, conf))
        return 6;

    /* listen addresses */
    if (0 != config_write_listen_addr(s, conf))
        return 7;

    /* chroot */
    if (0 != config_write_chroot(s, conf))
        return 8;

    /* usergroup */
    if (0 != config_write_usergroup(s, conf))
        return 9;

    /* end config data. */
    type = CONFIG_STREAM_TYPE_EOM;
    if (0 != ipc_write_uint8_block(s, type))
        return 10;

    /* success */
    return 0;
}

/**
 * \brief Write the log directory to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_write_logdir(int s, agent_config_t* conf)
{
    /* write the logdir to the stream if set. */
    if (NULL != conf->logdir)
    {
        /* write the logdir type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_LOGDIR;
        if (0 != ipc_write_uint8_block(s, type))
            return 1;

        /* write the logdir to the stream. */
        if (0 != ipc_write_string_block(s, conf->logdir))
            return 2;
    }

    /* success */
    return 0;
}

/**
 * \brief Write the loglevel to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_write_loglevel(int s, agent_config_t* conf)
{
    /* write the loglevel if set. */
    if (conf->loglevel_set)
    {
        /* write the loglevel_type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_LOGLEVEL;
        if (0 != ipc_write_uint8_block(s, type))
            return 1;

        /* write the loglevel to the stream. */
        if (0 != ipc_write_int64_block(s, conf->loglevel))
            return 2;
    }

    /* success. */
    return 0;
}

/**
 * \brief Write the secret to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_write_secret(int s, agent_config_t* conf)
{
    /* write the secret to the stream if set. */
    if (NULL != conf->secret)
    {
        /* write the secret type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_SECRET;
        if (0 != ipc_write_uint8_block(s, type))
            return 1;

        /* write the secret to the stream. */
        if (0 != ipc_write_string_block(s, conf->secret))
            return 2;
    }

    /* success. */
    return 0;
}

/**
 * \brief Write the rootblock to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_write_rootblock(int s, agent_config_t* conf)
{
    /* write the rootblock to the stream if set. */
    if (NULL != conf->rootblock)
    {
        /* write the rootblock type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_ROOTBLOCK;
        if (0 != ipc_write_uint8_block(s, type))
            return 1;

        /* write the rootblock to the stream. */
        if (0 != ipc_write_string_block(s, conf->rootblock))
            return 2;
    }

    /* success */
    return 0;
}

/**
 * \brief Write the datastore to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_write_datastore(int s, agent_config_t* conf)
{
    /* write the datastore to the stream if set. */
    if (NULL != conf->datastore)
    {
        /* write the datastore type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_DATASTORE;
        if (0 != ipc_write_uint8_block(s, type))
            return 1;

        /* write the datastore to the stream. */
        if (0 != ipc_write_string_block(s, conf->datastore))
            return 2;
    }

    /* success. */
    return 0;
}

/**
 * \brief Write the listen addresses to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_write_listen_addr(int s, agent_config_t* conf)
{
    /* Write all listen addresses to the stream. */
    config_listen_address_t* ptr = conf->listen_head;
    while (NULL != ptr)
    {
        /* write the listen address type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_LISTEN_ADDR;
        if (0 != ipc_write_uint8_block(s, type))
            return 1;

        /* convert the address to presentation format. */
        char addr[100];
        if (NULL == inet_ntop(AF_INET, ptr->addr, addr, sizeof(addr)))
            return 2;

        /* write the listen address to the stream. */
        if (0 != ipc_write_string_block(s, addr))
            return 3;

        /* write the listen port to the stream. */
        if (0 != ipc_write_uint64_block(s, ptr->port))
            return 4;

        /* skip to the next listen address. */
        ptr = (config_listen_address_t*)ptr->hdr.next;
    }

    /* success. */
    return 0;
}

/**
 * \brief Write the chroot to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_write_chroot(int s, agent_config_t* conf)
{
    /* write the chroot to the stream if set. */
    if (NULL != conf->chroot)
    {
        /* write the chroot type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_CHROOT;
        if (0 != ipc_write_uint8_block(s, type))
            return 1;

        /* write the chroot to the stream. */
        if (0 != ipc_write_string_block(s, conf->chroot))
            return 2;
    }

    /* success. */
    return 0;
}

/**
 * \brief Write the usergroup to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int config_write_usergroup(int s, agent_config_t* conf)
{
    /* write the user and group to the stream if set. */
    if (NULL != conf->usergroup)
    {
        /* write the usergroup type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_USERGROUP;
        if (0 != ipc_write_uint8_block(s, type))
            return 18;

        /* write the user to the stream. */
        if (0 != ipc_write_string_block(s, conf->usergroup->user))
            return 19;

        /* write the group to the stream. */
        if (0 != ipc_write_string_block(s, conf->usergroup->group))
            return 20;
    }

    /* success. */
    return 0;
}
