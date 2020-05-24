/**
 * \file authservice/auth_service_exit_event_loop.c
 *
 * \brief Make a clean exit from the event loop.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "auth_service_private.h"

/**
 * \brief Set up a clean re-entry from the event loop and ensure that no other
 * callbacks occur by setting the appropriate force exit flag.
 *
 * \param instance      The randomservice instance.
 */
void auth_service_exit_event_loop(auth_service_instance_t* instance)
{
    instance->auth_service_force_exit = true;
    ipc_exit_loop(instance->loop);
}
