/**
 * \file consensus/consensus_service_dataservice_sendreq_child_context_create.c
 *
 * \brief Send the child context create request to the data service from the
 * consensus service.
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
 * \brief Send a child context create request to the data service.
 *
 * \param instance      The consensus service instance.
 */
int consensus_service_dataservice_sendreq_child_context_create(
    consensusservice_instance_t* instance)
{
    BITCAP(dataservice_caps, DATASERVICE_API_CAP_BITS_MAX);

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != instance);

    /* wait on the child context create. */
    instance->state = CONSENSUS_SERVICE_STATE_WAITRESP_CHILD_CONTEXT_CREATE;

    /* set bitcaps based on the queries that the consensus service makes. */
    BITCAP_INIT_FALSE(dataservice_caps);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* send the request to open a child context. */
    int retval =
        dataservice_api_sendreq_child_context_create(
            instance->data, dataservice_caps, sizeof(dataservice_caps));
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        return retval;
    }

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        instance->data, &consensus_service_data_write, instance->loop_context);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
