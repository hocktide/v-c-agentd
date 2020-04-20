/**
 * \file
 * canonization/canonizationservice_dataservice_sendreq_child_context_create.c
 *
 * \brief Send the child context create request to the data service from the
 * canonization service.
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
 * \brief Send a child context create request to the data service.
 *
 * \param instance      The canonization service instance.
 */
int canonizationservice_dataservice_sendreq_child_context_create(
    canonizationservice_instance_t* instance)
{
    BITCAP(dataservice_caps, DATASERVICE_API_CAP_BITS_MAX);

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != instance);

    /* wait on the child context create. */
    instance->state = CANONIZATIONSERVICE_STATE_WAITRESP_CHILD_CONTEXT_CREATE;

    /* set bitcaps based on the queries that the canonization service makes. */
    BITCAP_INIT_FALSE(dataservice_caps);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(
        dataservice_caps, DATASERVICE_API_CAP_APP_BLOCK_READ);
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
        instance->data, &canonizationservice_data_write,
        instance->loop_context);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
