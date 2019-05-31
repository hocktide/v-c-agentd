/**
 * \file command/private_command_randomservice.c
 *
 * \brief Run the random service instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/randomservice.h>
#include <agentd/fds.h>
#include <cbmc/model_assert.h>
#include <vccrypt/suite.h>
#include <vpr/parameters.h>

/**
 * \brief Run the random service instance.
 */
void private_command_randomservice(bootstrap_config_t* UNUSED(bconf))
{
    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();

    /* run the event loop for the random service. */
    int retval =
        randomservice_event_loop(
            AGENTD_FD_RANDOM_SERVICE_RANDOM_DEVICE,
            AGENTD_FD_RANDOM_SERVICE_PROTOCOL_SERVICE,
            AGENTD_FD_RANDOM_SERVICE_LOG_SOCKET);

    /* exit with the return code from the event loop. */
    exit(retval);
}
