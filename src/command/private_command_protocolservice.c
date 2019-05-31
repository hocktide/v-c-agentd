/**
 * \file command/private_command_protocolservice.c
 *
 * \brief Run an unauthorized protocol service instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/protocolservice.h>
#include <agentd/fds.h>
#include <cbmc/model_assert.h>
#include <vccrypt/suite.h>
#include <vpr/parameters.h>

/**
 * \brief Run an unauthorized protocol service instance.
 */
void private_command_protocolservice(bootstrap_config_t* UNUSED(bconf))
{
    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();
    /* TODO - register sha-512/256 until the short hmac suite feature can be
     * merged into agentd. */
    vccrypt_mac_register_SHA_2_512_256_HMAC();

    /* run the event loop for the protocol service. */
    int retval =
        unauthorized_protocol_service_event_loop(
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_RANDOM,
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_ACCEPT,
            AGENTD_FD_UNAUTHORIZED_PROTOSVC_LOG);

    /* exit with the return code from the event loop. */
    exit(retval);
}
