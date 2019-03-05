/**
 * \file command/private_command_dataservice.c
 *
 * \brief Run a data service instance.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/dataservice.h>
#include <agentd/fds.h>
#include <cbmc/model_assert.h>
#include <vccrypt/suite.h>
#include <vpr/parameters.h>

/**
 * \brief Run a data service instance.
 */
void private_command_dataservice(bootstrap_config_t* UNUSED(bconf))
{
    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();

    /* run the event loop for the data service. */
    int retval =
        dataservice_event_loop(
            AGENTD_FD_DATASERVICE_SOCK, AGENTD_FD_DATASERVICE_LOG);

    /* exit with the return code from the event loop. */
    exit(retval);
}
