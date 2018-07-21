/**
 * \file commandline/parse_commandline_options.c
 *
 * \brief Parse commandline options, populating bootstrap config.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/commandline.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

/**
 * \brief Parse command-line options and set values in the bootstrap
 * configuration structure related to these options.
 *
 * \param bconf         The bootstrap config to update with these options.
 * \param argc          The number of arguments to parse.
 * \param argv          An array of command-line arguments.
 */
void parse_commandline_options(
    bootstrap_config_t* bconf, int argc, char** argv)
{
    int ch;

    MODEL_ASSERT(NULL != bconf);
    MODEL_ASSERT(1 <= argc);
    MODEL_ASSERT(NULL != argv);

    optind = 0;
#ifdef __OpenBSD__
    optreset = 1; /* in OpenBSD, we must also specify optreset. */
#endif

    while ((ch = getopt(argc, argv, "Fc:")) != -1)
    {
        switch (ch)
        {
            case 'F':
                bootstrap_config_set_foreground(bconf, true);
                break;

            case 'c':
                bootstrap_config_set_config_file(bconf, optarg);
                break;

            default:
                bootstrap_config_set_command(bconf, &command_error_usage);
                return;
        }
    }

    /* skip parsed options. */
    argc -= optind;
    argv += optind;

    commandline_dispatch_command(bconf, argc, argv);
}
