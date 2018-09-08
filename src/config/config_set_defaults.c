/**
 * \file config/config_set_defaults.c
 *
 * \brief Set defaults for config data.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/config.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

/**
 * \brief Set default values for any config setting that has not been set.
 *
 * \param conf          The config structure to populate.
 * \param bconf         The bootstrap config structure to use for overrides.
 *
 * \returns 0 on success and non-zero on failure.
 */
int config_set_defaults(agent_config_t* conf, bootstrap_config_t* bconf)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != conf);
    MODEL_ASSERT(NULL != bconf);

    /* if logdir is not set, set it to "/log" */
    if (NULL == conf->logdir)
    {
        /* attempt to set the logdir. */
        conf->logdir = strdup("/log");
        if (NULL == conf->logdir)
        {
            return 1;
        }
    }

    /* if loglevel is not set, set the log level to 4. */
    if (!conf->loglevel_set || conf->loglevel < 0 || conf->loglevel > 9)
    {
        conf->loglevel = 4;
        conf->loglevel_set = true;
    }

    /* if secret is not set, set it to "/root/secret.cert" */
    if (NULL == conf->secret)
    {
        /* attempt to set the secret. */
        conf->secret = strdup("/root/secret.cert");
        if (NULL == conf->secret)
        {
            return 2;
        }
    }

    /* if rootblock is not set, set it to "/root/root.cert" */
    if (NULL == conf->rootblock)
    {
        /* attempt to set the rootblock. */
        conf->rootblock = strdup("/root/root.cert");
        if (NULL == conf->rootblock)
        {
            return 3;
        }
    }

    /* if datastore is not set, set it to "/data" */
    if (NULL == conf->datastore)
    {
        /* attempt to set the datastore. */
        conf->datastore = strdup("/data");
        if (NULL == conf->datastore)
        {
            return 4;
        }
    }

    /* if there are no listen addresses, then listen to 0.0.0.0:4891 */
    if (NULL == conf->listen_head)
    {
        /* attempt to allocate memory for the node. */
        config_listen_address_t* node =
            (config_listen_address_t*)malloc(sizeof(config_listen_address_t));
        if (NULL == node)
        {
            return 5;
        }

        /* attempt to allocate memory for the address. */
        node->addr = (struct in_addr*)malloc(sizeof(struct in_addr));
        if (NULL == node->addr)
        {
            free(node);
            return 6;
        }

        /* set the bind address. */
        if (1 != inet_pton(AF_INET, "0.0.0.0", node->addr))
        {
            free(node->addr);
            free(node);
            return 7;
        }

        /* set the port. */
        node->port = 4891;

        /* there is no next listen address. */
        node->hdr.next = NULL;

        /* this node is the head of our listener list. */
        conf->listen_head = node;
    }

    /* the default chroot is our prefix. */
    if (NULL == conf->chroot)
    {
        /* attempt to set chroot. */
        conf->chroot = strdup(bconf->prefix_dir);
        if (NULL == conf->chroot)
        {
            return 8;
        }
    }

    /* the default user and group is "veloagent:veloagent" */
    if (NULL == conf->usergroup)
    {
        /* attempt to allocate a usergroup. */
        conf->usergroup =
            (config_user_group_t*)malloc(sizeof(config_user_group_t));
        if (NULL == conf->usergroup)
        {
            return 9;
        }

        /* attempt to set user.*/
        conf->usergroup->user = strdup("veloagent");
        if (NULL == conf->usergroup->user)
        {
            free(conf->usergroup);
            return 10;
        }

        /* attempt to set group. */
        conf->usergroup->group = strdup("veloagent");
        if (NULL == conf->usergroup->group)
        {
            free((char*)conf->usergroup->user);
            free(conf->usergroup);
            return 11;
        }
    }

    /* if we make it this far, all fields in conf are set. */
    MODEL_ASSERT(NULL != conf->logdir);
    MODEL_ASSERT(conf->loglevel_set);
    MODEL_ASSERT(conf->loglevel >= 0 && conf->loglevel <= 9);
    MODEL_ASSERT(NULL != conf->secret);
    MODEL_ASSERT(NULL != conf->rootblock);
    MODEL_ASSERT(NULL != conf->datastore);
    MODEL_ASSERT(NULL != conf->listen_head);
    MODEL_ASSERT(NULL != conf->chroot);
    MODEL_ASSERT(NULL != conf->usergroup);

    /* success. */
    return 0;
}
