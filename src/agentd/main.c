/**
 * \file agentd/main.c
 *
 * \brief Main entry point for the Velo Blockchain Agent.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/commandline.h>
#include <vpr/allocator/malloc_allocator.h>

int main(int argc, char** argv)
{
    bootstrap_config_t bconf;
    int retval = 0;

    /* initialize bootstrap config. */
    bootstrap_config_init(&bconf);

    /* attempt to parse command-line options. */
    retval = parse_commandline_options(&bconf, argc, argv);
    if (0 != retval)
    {
        goto cleanup_bconf;
    }

    /* do we have a command to execute? */
    if (NULL != bconf.command)
    {
        retval = bconf.command(&bconf);
    }
    else if (NULL != bconf.private_command)
    {
        /* we don't return here. */
        bconf.private_command();

        /* the compiler does not know this, so set a valid return value. */
        retval = 0;
    }
    else
    {
        fprintf(stderr, "Invalid configuration state.\n");
        retval = 1;
    }

cleanup_bconf:
    dispose((disposable_t*)&bconf);

    return retval;
}
