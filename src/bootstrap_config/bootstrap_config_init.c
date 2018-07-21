/**
 * \file bootstrap_config/bootstrap_config_init.c
 *
 * \brief Initialize an empty bootstrap_config structure.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <cbmc/model_assert.h>
#include <string.h>

/* forward decls */
static void bootstrap_config_dispose(void* disp);

/**
 * \brief Initialize bootstrap configuration.
 *
 * \param bconf         The bootstrap configuration data to initialize.
 */
void bootstrap_config_init(bootstrap_config_t* bconf)
{
    MODEL_ASSERT(NULL != bconf);

    /* clear out the config structure. */
    memset(bconf, 0, sizeof(bootstrap_config_t));

    /* set our dispose method. */
    bconf->hdr.dispose = &bootstrap_config_dispose;

    /* by default, the config file is located in etc/agentd.config. */
    bconf->config_file = strdup("etc/agentd.conf");
}

/**
 * \brief Dispose of a \ref bootstrap_config_t instance.
 *
 * \param disp      The structure to dispose.
 */
static void bootstrap_config_dispose(void* disp)
{
    bootstrap_config_t* bconf = (bootstrap_config_t*)disp;
    MODEL_ASSERT(NULL != bconf);
    MODEL_ASSERT(NULL != bconf->config_file);

    /* if the config_file is set, free it. */
    if (NULL != bconf->config_file)
        free((char*)bconf->config_file);
}
