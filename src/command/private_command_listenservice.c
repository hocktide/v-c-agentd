/**
 * \file command/private_command_listenservice.c
 *
 * \brief Run an listen service instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/listenservice.h>
#include <agentd/fds.h>
#include <cbmc/model_assert.h>
#include <vccrypt/suite.h>
#include <vpr/parameters.h>

/**
 * \brief Run a listen service instance.
 */
void private_command_listenservice(bootstrap_config_t* UNUSED(bconf))
{
    /* run the event loop for the listen service. */
    int retval =
        listenservice_event_loop(
            AGENTD_FD_LISTENSERVICE_LOG,
            AGENTD_FD_LISTENSERVICE_ACCEPT,
            AGENTD_FD_LISTENSERVICE_SOCK_START);

    /* exit with the return code from the event loop. */
    exit(retval);
}
