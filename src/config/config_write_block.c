/**
 * \file config/config_write_block.c
 *
 * \brief Write a config structure to the given stream.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <unistd.h>

/* forward decls */
static int config_write_logdir(int s, agent_config_t* conf);
static int config_write_loglevel(int s, agent_config_t* conf);
static int config_write_block_max_milliseconds(int s, agent_config_t* conf);
static int config_write_block_max_transactions(int s, agent_config_t* conf);
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
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE if writing data to the
 *        socket failed.
 *      - AGENTD_ERROR_CONFIG_INET_NTOP_FAILURE if converting the listen address
 *        to a string failed.
 */
int config_write_block(int s, agent_config_t* conf)
{
    int retval = 0;
    uint8_t type = 0U;

    /* begin config data. */
    type = CONFIG_STREAM_TYPE_BOM;
    if (AGENTD_STATUS_SUCCESS != ipc_write_uint8_block(s, type))
        return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

    /* logdir */
    retval = config_write_logdir(s, conf);
    if (AGENTD_STATUS_SUCCESS != retval)
        return retval;

    /* loglevel */
    retval = config_write_loglevel(s, conf);
    if (AGENTD_STATUS_SUCCESS != retval)
        return retval;

    /* block max milliseconds */
    retval = config_write_block_max_milliseconds(s, conf);
    if (AGENTD_STATUS_SUCCESS != retval)
        return retval;

    /* block max transactions */
    retval = config_write_block_max_transactions(s, conf);
    if (AGENTD_STATUS_SUCCESS != retval)
        return retval;

    /* secret */
    retval = config_write_secret(s, conf);
    if (AGENTD_STATUS_SUCCESS != retval)
        return retval;

    /* rootblock */
    retval = config_write_rootblock(s, conf);
    if (AGENTD_STATUS_SUCCESS != retval)
        return retval;

    /* datastore */
    retval = config_write_datastore(s, conf);
    if (AGENTD_STATUS_SUCCESS != retval)
        return retval;

    /* listen addresses */
    retval = config_write_listen_addr(s, conf);
    if (AGENTD_STATUS_SUCCESS != retval)
        return retval;

    /* chroot */
    retval = config_write_chroot(s, conf);
    if (AGENTD_STATUS_SUCCESS != retval)
        return retval;

    /* usergroup */
    retval = config_write_usergroup(s, conf);
    if (AGENTD_STATUS_SUCCESS != retval)
        return retval;

    /* end config data. */
    type = CONFIG_STREAM_TYPE_EOM;
    if (AGENTD_STATUS_SUCCESS != ipc_write_uint8_block(s, type))
        return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Write the log directory to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE if writing data to the
 *        socket failed.
 */
static int config_write_logdir(int s, agent_config_t* conf)
{
    /* write the logdir to the stream if set. */
    if (NULL != conf->logdir)
    {
        /* write the logdir type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_LOGDIR;
        if (AGENTD_STATUS_SUCCESS != ipc_write_uint8_block(s, type))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* write the logdir to the stream. */
        if (AGENTD_STATUS_SUCCESS != ipc_write_string_block(s, conf->logdir))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;
    }

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Write the loglevel to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE if writing data to the
 *        socket failed.
 */
static int config_write_loglevel(int s, agent_config_t* conf)
{
    /* write the loglevel if set. */
    if (conf->loglevel_set)
    {
        /* write the loglevel_type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_LOGLEVEL;
        if (AGENTD_STATUS_SUCCESS != ipc_write_uint8_block(s, type))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* write the loglevel to the stream. */
        if (AGENTD_STATUS_SUCCESS != ipc_write_int64_block(s, conf->loglevel))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Write the block max milliseconds to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE if writing data to the
 *        socket failed.
 */
static int config_write_block_max_milliseconds(int s, agent_config_t* conf)
{
    /* write the block max milliseconds if set. */
    if (conf->block_max_milliseconds_set)
    {
        /* write the block max milliseconds type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_BLOCK_MAX_MILLISECONDS;
        if (AGENTD_STATUS_SUCCESS != ipc_write_uint8_block(s, type))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* write the block max milliseconds to the stream. */
        if (AGENTD_STATUS_SUCCESS !=
            ipc_write_int64_block(s, conf->block_max_milliseconds))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Write the block max transactions to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE if writing data to the
 *        socket failed.
 */
static int config_write_block_max_transactions(int s, agent_config_t* conf)
{
    /* write the block max transactions if set. */
    if (conf->block_max_transactions_set)
    {
        /* write the block max transactions type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_BLOCK_MAX_TRANSACTIONS;
        if (AGENTD_STATUS_SUCCESS != ipc_write_uint8_block(s, type))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* write the block max transactions to the stream. */
        if (AGENTD_STATUS_SUCCESS !=
            ipc_write_int64_block(s, conf->block_max_transactions))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Write the secret to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE if writing data to the
 *        socket failed.
 */
static int config_write_secret(int s, agent_config_t* conf)
{
    /* write the secret to the stream if set. */
    if (NULL != conf->secret)
    {
        /* write the secret type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_SECRET;
        if (AGENTD_STATUS_SUCCESS != ipc_write_uint8_block(s, type))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* write the secret to the stream. */
        if (AGENTD_STATUS_SUCCESS != ipc_write_string_block(s, conf->secret))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Write the rootblock to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE if writing data to the
 *        socket failed.
 */
static int config_write_rootblock(int s, agent_config_t* conf)
{
    /* write the rootblock to the stream if set. */
    if (NULL != conf->rootblock)
    {
        /* write the rootblock type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_ROOTBLOCK;
        if (AGENTD_STATUS_SUCCESS != ipc_write_uint8_block(s, type))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* write the rootblock to the stream. */
        if (AGENTD_STATUS_SUCCESS != ipc_write_string_block(s, conf->rootblock))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;
    }

    /* success */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Write the datastore to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE if writing data to the
 *        socket failed.
 */
static int config_write_datastore(int s, agent_config_t* conf)
{
    /* write the datastore to the stream if set. */
    if (NULL != conf->datastore)
    {
        /* write the datastore type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_DATASTORE;
        if (AGENTD_STATUS_SUCCESS != ipc_write_uint8_block(s, type))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* write the datastore to the stream. */
        if (AGENTD_STATUS_SUCCESS != ipc_write_string_block(s, conf->datastore))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Write the listen addresses to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE if writing data to the
 *        socket failed.
 *      - AGENTD_ERROR_CONFIG_INET_NTOP_FAILURE if converting the listen address
 *        to a string failed.
 */
static int config_write_listen_addr(int s, agent_config_t* conf)
{
    /* Write all listen addresses to the stream. */
    config_listen_address_t* ptr = conf->listen_head;
    while (NULL != ptr)
    {
        /* write the listen address type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_LISTEN_ADDR;
        if (AGENTD_STATUS_SUCCESS != ipc_write_uint8_block(s, type))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* convert the address to presentation format. */
        char addr[100];
        if (NULL == inet_ntop(AF_INET, ptr->addr, addr, sizeof(addr)))
            return AGENTD_ERROR_CONFIG_INET_NTOP_FAILURE;

        /* write the listen address to the stream. */
        if (AGENTD_STATUS_SUCCESS != ipc_write_string_block(s, addr))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* write the listen port to the stream. */
        if (AGENTD_STATUS_SUCCESS != ipc_write_uint64_block(s, ptr->port))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* skip to the next listen address. */
        ptr = (config_listen_address_t*)ptr->hdr.next;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Write the chroot to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE if writing data to the
 *        socket failed.
 */
static int config_write_chroot(int s, agent_config_t* conf)
{
    /* write the chroot to the stream if set. */
    if (NULL != conf->chroot)
    {
        /* write the chroot type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_CHROOT;
        if (AGENTD_STATUS_SUCCESS != ipc_write_uint8_block(s, type))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* write the chroot to the stream. */
        if (AGENTD_STATUS_SUCCESS != ipc_write_string_block(s, conf->chroot))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Write the usergroup to the config output stream.
 *
 * \param s             The config output stream.
 * \param conf          The config structure from which this value is obtained.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE if writing data to the
 *        socket failed.
 */
static int config_write_usergroup(int s, agent_config_t* conf)
{
    /* write the user and group to the stream if set. */
    if (NULL != conf->usergroup)
    {
        /* write the usergroup type to the stream. */
        uint8_t type = CONFIG_STREAM_TYPE_USERGROUP;
        if (AGENTD_STATUS_SUCCESS != ipc_write_uint8_block(s, type))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* write the user to the stream. */
        if (AGENTD_STATUS_SUCCESS != ipc_write_string_block(s, conf->usergroup->user))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;

        /* write the group to the stream. */
        if (AGENTD_STATUS_SUCCESS != ipc_write_string_block(s, conf->usergroup->group))
            return AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
