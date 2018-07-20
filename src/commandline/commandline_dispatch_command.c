/**
 * \file commandline/commandline_dispatch_command.c
 *
 * \brief Dispatch a command specified in the commandline.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/commandline.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <vpr/parameters.h>

/**
 * \brief Dispatch a command.
 *
 * \param bconf         The bootstrap config.
 * \param argc          The number of command arguments.
 * \param argv          The command argument vector.
 *
 * \returns 0 on success and non-zero on failure.
 */
int commandline_dispatch_command(
    bootstrap_config_t* UNUSED(bconf), int UNUSED(argc), char** argv)
{
    MODEL_ASSERT(NULL != bconf);
    MODEL_ASSERT(argc >= 1);
    MODEL_ASSERT(NULL != argv);

    if (!strcmp(argv[0], "help"))
    {
        return commandline_print_usage(stdout, 0);
    }
    else
    {
        fprintf(stderr, "Unknown command '%s'.\n", argv[0]);

        return commandline_print_usage(stderr, 1);
    }
}
