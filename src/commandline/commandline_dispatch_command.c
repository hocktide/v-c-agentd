/**
 * \file commandline/commandline_dispatch_command.c
 *
 * \brief Dispatch a command specified in the commandline.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
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
 */
void commandline_dispatch_command(
    bootstrap_config_t* bconf, int argc, char** argv)
{
    MODEL_ASSERT(NULL != bconf);
    MODEL_ASSERT(argc >= 1);
    MODEL_ASSERT(NULL != argv);

    /* verify that we have at least one argument. */
    if (argc < 1)
    {
        fprintf(stderr, "Expecting command.\n\n");
        bootstrap_config_set_command(bconf, &command_error_usage);
    }
    /* help command. */
    else if (!strcmp(argv[0], "help"))
    {
        bootstrap_config_set_command(bconf, &command_help);
    }
    /* readconfig command. */
    else if (!strcmp(argv[0], "readconfig"))
    {
        bootstrap_config_set_command(bconf, &command_readconfig);
    }
    else
    {
        fprintf(stderr, "Unknown command '%s'.\n\n", argv[0]);
        bootstrap_config_set_command(bconf, &command_error_usage);
    }
}
