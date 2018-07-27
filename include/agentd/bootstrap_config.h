/**
 * \file agentd/bootstrap_config.h
 *
 * \brief Configuration data structure for bootstrapping agentd.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_BOOTSTRAP_CONFIG_HEADER_GUARD
#define AGENTD_BOOTSTRAP_CONFIG_HEADER_GUARD

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/* forward decl for bootstrap config structure. */
struct bootstrap_config;

/**
 * \brief Commands execute functions within the service.
 *
 * \param bconf         The bootstrap configuration for this command.
 *
 * \returns 0 on success and non-zero on failure.  May not return.
 */
typedef int (*bootstrap_config_command_t)(struct bootstrap_config* bconf);

/**
 * \brief Private commands are used to execute specific privilege separated
 * services in agentd.
 */
typedef void (*bootstrap_config_private_command_t)();

/**
 * \brief The \ref bootstrap_config_t type provides a structure for storing
 * config values from the command-line.
 */
typedef struct bootstrap_config
{
    /** \brief This config is disposable. */
    disposable_t hdr;

    /** \brief Run the service in the foreground (don't daemonize). */
    bool foreground;

    /** \brief Command to execute. */
    bootstrap_config_command_t command;

    /** \brief Private (privsep) command to execute. */
    bootstrap_config_private_command_t private_command;

    /** \brief Config file location. */
    const char* config_file;

    /** \brief Absolute location of the binary. */
    const char* binary;

    /** \brief Prefix directory. */
    const char* prefix_dir;
} bootstrap_config_t;

/**
 * \brief Initialize bootstrap configuration.
 *
 * \param bconf         The bootstrap configuration data to initialize.
 */
void bootstrap_config_init(bootstrap_config_t* bconf);

/**
 * \brief Set agentd to run in the foreground (true) or background (false).
 *
 * \param bconf         The bootstrap configuration data to update.
 * \param foreground    Set to true: run in foreground.  Set to false: run in
 *                      background.
 */
void bootstrap_config_set_foreground(
    bootstrap_config_t* bconf, bool foreground);

/**
 * \brief Set agentd to run the given command.
 *
 * \param bconf         The bootstrap configuration data to update.
 * \param command       The command to run.
 */
void bootstrap_config_set_command(
    bootstrap_config_t* bconf, bootstrap_config_command_t command);

/**
 * \brief Set agentd to run the given private (privsep) command.
 *
 * \param bconf         The bootstrap configuration data to update.
 * \param command       The private command to run.
 */
void bootstrap_config_set_private_command(
    bootstrap_config_t* bconf, bootstrap_config_private_command_t command);

/**
 * \brief Set the config file for agentd.  The string value is copied; the
 * caller retains ownership of the original string, and the
 * \ref bootstrap_config_t structure maintains its own copy.
 *
 * \param bconf         The bootstrap configuration data to update.
 * \param config        The config file to read.
 */
void bootstrap_config_set_config_file(
    bootstrap_config_t* bconf, const char* config_file);

/**
 * \brief Set the binary name for agentd.
 *
 * The string value provided, typically as argv[0], is canonicalized and used as
 * the basis of the real path to this binary.  On success, the binary name is
 * updated.  On failure, this method returns a non-zero status.  The caller
 * retains ownership of the bname parameter; this method makes an internal copy
 * of the canonical name, which is freed when this bootstrap config structure is
 * dispose()d.
 *
 * \param bconf         The bootstrap configuration data to update.
 * \param bname         The name of the binary to be canonicalized and set.
 */
int bootstrap_config_set_binary(
    bootstrap_config_t* bconf, const char* bname);

/**
 * \brief Resolve the prefix directory using the binary name.
 *
 * This method can only be called after \ref bootstrap_config_set_binary() was
 * called and returned a successful status code.  This method sets the prefix
 * directory based on the binary name.  This prefix directory is used for
 * creating a temporary \ref chroot() so the config file can be read.
 *
 * \param bconf         The bootstrap configuration data to update.
 */
int bootstrap_config_resolve_prefix_dir(
    bootstrap_config_t* bconf);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_BOOTSTRAP_CONFIG_HEADER_GUARD*/
