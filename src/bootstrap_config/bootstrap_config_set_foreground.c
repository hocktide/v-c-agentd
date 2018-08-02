/**
 * \file bootstrap_config/bootstrap_config_set_foreground.c
 *
 * \brief Set the foreground flag in the bootstrap config.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <cbmc/model_assert.h>
#include <string.h>

/**
 * \brief Set agentd to run in the foreground (true) or background (false).
 *
 * \param bconf         The bootstrap configuration data to update.
 * \param foreground    Set to true: run in foreground.  Set to false: run in
 *                      background.
 */
void bootstrap_config_set_foreground(
    bootstrap_config_t* bconf, bool foreground)
{
    MODEL_ASSERT(NULL != bconf);
    bconf->foreground = foreground;
}
