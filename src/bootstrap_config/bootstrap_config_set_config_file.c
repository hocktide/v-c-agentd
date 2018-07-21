/**
 * \file bootstrap_config/bootstrap_config_set_config_file.c
 *
 * \brief Set the config file for the agentd process.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <cbmc/model_assert.h>
#include <string.h>

/**
 * \brief Set the config file for agentd.  The string value is copied; the
 * caller retains ownership of the original string, and the
 * \ref bootstrap_config_t structure maintains its own copy.
 *
 * \param bconf         The bootstrap configuration data to update.
 * \param config        The config file to read.
 */
void bootstrap_config_set_config_file(
    bootstrap_config_t* bconf, const char* config_file)
{
    MODEL_ASSERT(NULL != bconf);
    MODEL_ASSERT(NULL != config_file);

    if (NULL != bconf->config_file)
        free((char*)bconf->config_file);

    bconf->config_file = strdup(config_file);
}
