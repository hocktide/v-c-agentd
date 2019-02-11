/**
 * \file commandline/commandline_dispatch_private_command.c
 *
 * \brief Dispatch a private command specified in the command argument.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/commandline.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <vpr/parameters.h>

/**
 * \brief Dispatch a private command.
 *
 * \param bconf         The bootstrap config.
 * \param command       The name of the command to dispatch.
 */
void commandline_dispatch_private_command(
    bootstrap_config_t* bconf, const char* command)
{
    MODEL_ASSERT(NULL != bconf);
    MODEL_ASSERT(NULL != command);

    /* is this the readconfig command? */
    if (!strcmp(command, "readconfig"))
    {
        bootstrap_config_set_private_command(
            bconf, private_command_readconfig);
    }
    /* is this the dataservice private command? */
    else if (!strcmp(command, "dataservice"))
    {
        bootstrap_config_set_private_command(
            bconf, private_command_dataservice);
    }
    else if (!strcmp(command, "unauthorized_protocol_service"))
    {
        bootstrap_config_set_private_command(
            bconf, private_command_protocolservice);
    }
    /* is this the supervisor command? */
    else if (!strcmp(command, "supervisor"))
    {
        bootstrap_config_set_private_command(
            bconf, private_command_supervisor);
    }
    else
    {
        /* indicate that there was an error, but -P is undocumented, so don't
         * provide any more hints. Users should not be using -P. */

        fprintf(stderr, "Invalid option.\n\n");
        bootstrap_config_set_command(
            bconf, &command_error_usage);
    }
}
