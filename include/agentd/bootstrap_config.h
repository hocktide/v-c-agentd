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

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_BOOTSTRAP_CONFIG_HEADER_GUARD*/
