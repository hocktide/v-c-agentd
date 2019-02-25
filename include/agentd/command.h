/**
 * \file agentd/command.h
 *
 * \brief Commands supported by agentd.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_COMMAND_HEADER_GUARD
#define AGENTD_COMMAND_HEADER_GUARD

#include <agentd/bootstrap_config.h>
#include <agentd/commandline.h>
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
 * \brief Read and verify the config file, writing human readable settings to
 * standard output.
 *
 * \param bconf         The bootstrap configuration for this command.
 *
 * \returns 0 on success and non-zero on failure.
 */
int command_readconfig(struct bootstrap_config* bconf);

/**
 * \brief Read the config file from a stream.
 */
void private_command_readconfig();

/**
 * \brief Run a data service instance.
 */
void private_command_dataservice();

/**
 * \brief Run a listen service instance.
 */
void private_command_listenservice();

/**
 * \brief Run an unauthorized protocol service instance.
 */
void private_command_protocolservice();

/**
 * \brief Run the the supervisor.
 */
void private_command_supervisor();

/**
 * \brief Start the blockchain agent.
 *
 * \param bconf         The bootstrap configuration for this command.
 *
 * \returns 0 on success and non-zero on failure.
 */
int command_start(struct bootstrap_config* bconf);

/**
 * \brief Print help information to standard output.
 *
 * \param bconf         The bootstrap configuration for this command.
 *
 * \returns 0 on success and non-zero on failure.
 */
int command_help(struct bootstrap_config* bconf);

/**
 * \brief Print help information to standard error and exit with an error code.
 *
 * \param bconf         The bootstrap configuration for this command.
 *
 * \returns non-zero error code.
 */
int command_error_usage(struct bootstrap_config* bconf);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_COMMAND_HEADER_GUARD*/
