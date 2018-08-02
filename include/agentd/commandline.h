/**
 * \file agentd/commandline.h
 *
 * \brief Functionality related to parsing commandline options.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_COMMANDLINE_HEADER_GUARD
#define AGENTD_COMMANDLINE_HEADER_GUARD

#include <agentd/bootstrap_config.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Parse command-line options and set values in the bootstrap
 * configuration structure related to these options.
 *
 * \param bconf         The bootstrap config to update with these options.
 * \param argc          The number of arguments to parse.
 * \param argv          An array of command-line arguments.
 */
void parse_commandline_options(
    bootstrap_config_t* bconf, int argc, char** argv);

/**
 * \brief Print usage and return the given response code.
 *
 * \param out           The file handle to use for output.
 * \param returncode    The return code to return to the caller.
 *
 * \returns 0 on success and non-zero on failure.
 */
int commandline_print_usage(FILE* out, int returncode);

/**
 * \brief Dispatch a command.
 *
 * \param bconf         The bootstrap config.
 * \param argc          The number of command arguments.
 * \param argv          The command argument vector.
 */
void commandline_dispatch_command(
    bootstrap_config_t* bconf, int argc, char** argv);

/**
 * \brief Dispatch a private command.
 *
 * \param bconf         The bootstrap config.
 * \param command       The name of the command to dispatch.
 */
void commandline_dispatch_private_command(
    bootstrap_config_t* bconf, const char* command);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_COMMANDLINE_HEADER_GUARD*/
