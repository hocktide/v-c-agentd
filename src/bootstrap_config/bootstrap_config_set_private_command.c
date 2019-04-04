/**
 * \file bootstrap_config/bootstrap_config_set_private_command.c
 *
 * \brief Set the private (privsep) startup command in the bootstrap config.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

/**
 * \brief Set agentd to run the given private (privsep) command.
 *
 * \param bconf         The bootstrap configuration data to update.
 * \param command       The private command to run.
 */
void bootstrap_config_set_private_command(
    bootstrap_config_t* bconf, bootstrap_config_private_command_t command)
{
    MODEL_ASSERT(NULL != bconf);
    MODEL_ASSERT(NULL != command);

    bconf->private_command = command;
}
