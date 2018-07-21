/**
 * \file command/command_help.c
 *
 * \brief Print help information to standard output.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Print help information to standard output.
 *
 * \param bconf         The bootstrap configuration for this command.
 *
 * \returns 0 on success and non-zero on failure.
 */
int command_help(struct bootstrap_config* UNUSED(bconf))
{
    MODEL_ASSERT(NULL != bconf);

    return commandline_print_usage(stdout, 0);
}
