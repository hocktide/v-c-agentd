/**
 * \file command/private_command_consensus_service.c
 *
 * \brief Run a consensus service instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/consensusservice.h>
#include <agentd/fds.h>
#include <cbmc/model_assert.h>
#include <vccrypt/suite.h>
#include <vpr/parameters.h>

/**
 * \brief Run a consensus service instance.
 */
void private_command_consensus_service(bootstrap_config_t* UNUSED(bconf))
{
    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();

    /* run the event loop for the consensus service. */
    int retval =
        consensus_service_event_loop(
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_DATA,
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_LOG);

    /* exit with the return code from the event loop. */
    exit(retval);
}
