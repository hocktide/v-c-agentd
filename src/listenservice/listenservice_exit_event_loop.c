/**
 * \file listenservice/listenservice_exit_event_loop.c
 *
 * \brief Make a clean exit from the event loop.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <cbmc/model_assert.h>

#include "listenservice_internal.h"

/**
 * \brief Set up a clean re-entry from the event loop and ensure that no other
 * callbacks occur by setting the appropriate force exit flag.
 *
 * \param instance      The listenservice instance.
 */
void listenservice_exit_event_loop(listenservice_instance_t* instance)
{
    instance->listenservice_force_exit = true;
    ipc_exit_loop(instance->loop_context);
}
