/**
 * \file consensus/consensus_service_timer_cb.c
 *
 * \brief Close the child context, leading to reset of the consensus service.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/consensusservice.h>
#include <agentd/consensusservice/api.h>
#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "consensusservice_internal.h"

/**
 * \brief Close the child context, leading to reset of the consensus service.
 *
 * \param instance      The consensus service instance.
 */
void consensus_service_child_context_close(
    consensusservice_instance_t* instance)
{
    /* close the child context. */
    int retval =
        dataservice_api_sendreq_child_context_close(
            instance->data, instance->data_child_context);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        return;
    }

    /* wait for the child context to close. */
    instance->state = CONSENSUS_SERVICE_STATE_WAITRESP_CHILD_CONTEXT_CLOSE;

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        instance->data, &consensus_service_data_write, instance->loop_context);
}
