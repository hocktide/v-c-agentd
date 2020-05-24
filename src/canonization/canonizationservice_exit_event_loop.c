/**
 * \file canonization/canonizationservice_exit_event_loop.c
 *
 * \brief Make a clean exit from the event loop.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Set up a clean re-entry from the event loop and ensure that no other
 * callbacks occur by setting the appropriate force exit flag.
 *
 * \param instance      The canonization service instance.
 */
void canonizationservice_exit_event_loop(
    canonizationservice_instance_t* instance)
{
    instance->force_exit = true;
    ipc_exit_loop(instance->loop_context);
}
