/**
 * \file protocolservice/unauthorized_protocol_service_exit_event_loop.c
 *
 * \brief Cleanly exit the event loop on fatal error.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Set up a clean re-entry from the event loop and ensure that no other
 * callbacks occur by setting the appropriate force exit flag.
 *
 * \param instance      The unauthorized protocol service instance.
 */
void unauthorized_protocol_service_exit_event_loop(
    unauthorized_protocol_service_instance_t* instance)
{
    instance->force_exit = true;
    ipc_exit_loop(&instance->loop);
}
