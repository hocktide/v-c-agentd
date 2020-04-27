/**
 * \file bootstrap_config/bootstrap_config_set_init_mode.c
 *
 * \brief Set the init_mode flag in the bootstrap config.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <cbmc/model_assert.h>
#include <string.h>

/**
 * \brief Set agentd to start in init mode (exec but don't fork).
 *
 * \param bconf         The bootstrap configuration data to update.
 * \param init_mode     Set to true: run in init mode.  Set to false: run in
 *                      forked mode.
 */
void bootstrap_config_set_init_mode(
    bootstrap_config_t* bconf, bool init_mode)
{
    MODEL_ASSERT(NULL != bconf);
    bconf->init_mode = init_mode;
}
