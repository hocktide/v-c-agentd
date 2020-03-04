/**
 * \file consensus/consensus_service_reset.c
 *
 * \brief Reset the consensus service for the next timer event.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/consensusservice.h>
#include <agentd/consensusservice/api.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "consensusservice_internal.h"

/**
 * \brief Clean up and reset the consensus service.
 *
 * \param instance      The consensus service instance.
 * \param should_sleep  If set, wake up on the sleep timer.  If not set, call
 *                      the sleep timer callback right away.
 */
void consensus_service_reset(
    consensusservice_instance_t* instance, bool should_sleep)
{
    int retval;

    /* set the state to idle. */
    instance->state = CONSENSUS_SERVICE_STATE_IDLE;

    /* clear the block id. */
    memset(instance->block_id, 0, sizeof(instance->block_id));

    /* dispose the transaction list. */
    dispose((disposable_t*)instance->transaction_list);
    memset(instance->transaction_list, 0, sizeof(linked_list_t));
    free(instance->transaction_list);
    instance->transaction_list = NULL;

    if (should_sleep)
    {
        /* dispose the old timer. */
        dispose((disposable_t*)&instance->timer);

        /* create the new timer. */
        retval =
            ipc_timer_init(
                &instance->timer, instance->block_max_milliseconds,
                &consensus_service_timer_cb, instance);
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            ipc_exit_loop(instance->loop_context);
            goto done;
        }

        /* set the timer event. */
        retval =
            ipc_event_loop_add_timer(instance->loop_context, &instance->timer);
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            ipc_exit_loop(instance->loop_context);
            goto cleanup_timer;
        }
    }
    else
    {
        /* call the callback immediately if there are more transactions to
         * process. */
        consensus_service_timer_cb(&instance->timer, instance);
    }

    goto done;

cleanup_timer:
    dispose((disposable_t*)&instance->timer);

done:;
}
