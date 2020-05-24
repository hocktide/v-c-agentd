/**
 * \file dataservice/randomservice_exit_event_loop.c
 *
 * \brief Make a clean exit from the event loop.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/private/randomservice.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "randomservice_internal.h"

/**
 * \brief Set up a clean re-entry from the event loop and ensure that no other
 * callbacks occur by setting the appropriate force exit flag.
 *
 * \param instance      The randomservice instance.
 */
void randomservice_exit_event_loop(randomservice_root_context_t* instance)
{
    instance->randomservice_force_exit = true;
    ipc_exit_loop(instance->loop_context);
}
