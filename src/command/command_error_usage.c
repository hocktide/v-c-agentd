/**
 * \file command/command_error_usage.c
 *
 * \brief Print help information to standard error and exit with an error code.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Print help information to standard error and exit with an error code.
 *
 * \param bconf         The bootstrap configuration for this command.
 *
 * \returns non-zero error code.
 */
int command_error_usage(struct bootstrap_config* UNUSED(bconf))
{
    MODEL_ASSERT(NULL != bconf);

    return commandline_print_usage(stderr, 1);
}
