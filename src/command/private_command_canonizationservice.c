/**
 * \file command/private_command_canonizationservice.c
 *
 * \brief Run a canonization service instance.
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/canonizationservice.h>
#include <agentd/fds.h>
#include <cbmc/model_assert.h>
#include <vccrypt/suite.h>
#include <vpr/parameters.h>

/**
 * \brief Run a canonization service instance.
 */
void private_command_canonizationservice(bootstrap_config_t* UNUSED(bconf))
{
    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();

    /* run the event loop for the canonization service. */
    int retval =
        canonizationservice_event_loop(
            AGENTD_FD_CANONIZATION_SVC_DATA,
            AGENTD_FD_CANONIZATION_SVC_RANDOM,
            AGENTD_FD_CANONIZATION_SVC_LOG,
            AGENTD_FD_CANONIZATION_SVC_CONTROL);

    /* exit with the return code from the event loop. */
    exit(retval);
}
