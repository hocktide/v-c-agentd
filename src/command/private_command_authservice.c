/**
 * \file command/private_command_authservice.c
 *
 * \brief Run an auth service instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/authservice.h>
#include <agentd/fds.h>
#include <vccrypt/suite.h>
#include <vpr/parameters.h>

/**
 * \brief Run an auth service instance.
 */
void private_command_authservice(bootstrap_config_t* UNUSED(bconf))
{
    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();

    /* run the event loop for the auth service. */
    int retval =
        auth_service_event_loop(
            AGENTD_FD_AUTHSERVICE_SOCK,
            AGENTD_FD_AUTHSERVICE_LOG);

    /* exit with the return code from the event loop. */
    exit(retval);
}
