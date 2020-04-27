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

    /* reset the option indicator. */
    optind = 0;
#ifdef __OpenBSD__
    optreset = 1; /* in OpenBSD, we must also specify optreset. */
#endif

    /* read through command-line options. */
    while ((ch = getopt(argc, argv, "FIP:c:")) != -1)
    {
        switch (ch)
        {
            /* run the agent in the foreground. */
            case 'F':
                bootstrap_config_set_foreground(bconf, true);
                break;

            /* run the agent in init mode. */
            case 'I':
                bootstrap_config_set_init_mode(bconf, true);
                break;

            /* run a private (privsep) command. */
            case 'P':
                commandline_dispatch_private_command(bconf, optarg);
                break;

            /* override the config file location. */
            case 'c':
                bootstrap_config_set_config_file(bconf, optarg);
                break;

            /* unknown option. */
            default:
                bootstrap_config_set_command(bconf, &command_error_usage);
                return;
        }
    }

    /* only dispatch a command if a private command has not been set, and an
      * error command has not already been set. */
    if (NULL == bconf->private_command && NULL == bconf->command)
    {
        /* skip parsed options. */
        argc -= optind;
        argv += optind;

        commandline_dispatch_command(bconf, argc, argv);
    }
}
