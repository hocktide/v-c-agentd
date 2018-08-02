/**
 * \file agentd/main.c
 *
 * \brief Main entry point for the Velo Blockchain Agent.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/commandline.h>
#include <stdio.h>
#include <vpr/allocator/malloc_allocator.h>

/* forward decls */
static int create_bootstrap_config(bootstrap_config_t* bconf, char* progname);

/**
 * \brief Main entry point.
 *
 * \param argc          Number of arguments.
 * \param argv          List of arguments.
 *
 * \returns 0 on successful execution, and non-zero on failure.
 */
int main(int argc, char** argv)
{
    bootstrap_config_t bconf;
    int retval = 0;

    /* initialize bootstrap config. */
    retval = create_bootstrap_config(&bconf, argv[0]);
    if (0 != retval)
    {
        return retval;
    }

    /* parse command-line options. */
    parse_commandline_options(&bconf, argc, argv);

    /* do we have a command to execute? */
    if (NULL != bconf.command)
    {
        /* execute the command. */
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
        /* this should not happen. */
        fprintf(stderr, "Invalid configuration state.\n");
        retval = 1;
    }

    /* clean up bootstrap config. */
    dispose((disposable_t*)&bconf);

    return retval;
}

/**
 * \brief Create the bootstrap config and manage binary / installation prefix
 * resolution.
 *
 * \param bconf         The \ref bootstrap_config_t instance to configure.
 * \param progname      The program name.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int create_bootstrap_config(bootstrap_config_t* bconf, char* progname)
{
    int retval = 0;

    /* initialize bootstrap config. */
    bootstrap_config_init(bconf);

    /* get the axecutable location. */
    retval = bootstrap_config_set_binary(bconf, progname);
    if (0 != retval)
    {
        fprintf(stderr, "Could not get absolute path to agentd binary.\n");
        goto cleanup_bconf;
    }

    /* resolve the prefix directory using the binary name. */
    retval = bootstrap_config_resolve_prefix_dir(bconf);
    if (0 != retval)
    {
        fprintf(stderr, "Could not resolve the installation prefix.\n");
        goto cleanup_bconf;
    }

    /* success. */
    return retval;

cleanup_bconf:
    dispose((disposable_t*)bconf);

    return retval;
}
