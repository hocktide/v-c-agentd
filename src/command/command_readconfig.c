/**
 * \file command/command_readconfig.c
 *
 * \brief Read and verify the config file, writing human readable settings to
 * standard output.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Read and verify the config file, writing human readable settings to
 * standard output.
 *
 * \param bconf         The bootstrap configuration for this command.
 *
 * \returns 0 on success and non-zero on failure.
 */
int command_readconfig(struct bootstrap_config* UNUSED(bconf))
{
    return 1;
}
