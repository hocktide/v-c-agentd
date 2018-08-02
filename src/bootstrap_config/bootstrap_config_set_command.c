/**
 * \file bootstrap_config/bootstrap_config_set_command.c
 *
 * \brief Set the startup command in the bootstrap config.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <cbmc/model_assert.h>

/**
 * \brief Set agentd to run the given command.
 *
 * \param bconf         The bootstrap configuration data to update.
 * \param command       The command to run.
 */
void bootstrap_config_set_command(
    bootstrap_config_t* bconf, bootstrap_config_command_t command)
{
    MODEL_ASSERT(NULL != bconf);

    bconf->command = command;
}
