/**
 * \file * canonization/canonizationservice_dataservice_sendreq_block_get.c
 *
 * \brief Send the block get by id request to the data service from the
 * canonization service.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

#include "canonizationservice_internal.h"

/**
 * \brief Send a request to get a block by id from the data service.
 *
 * \param instance      The canonization service instance.
 */
int canonizationservice_dataservice_sendreq_block_get(
    canonizationservice_instance_t* instance)
{
    int retval;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != instance);

    /* evolve the state of the canonization service; we now want to read the
     * latest block id. */
    instance->state = CANONIZATIONSERVICE_STATE_WAITRESP_BLOCK_GET;

    /* send the request to read the previous block. */
    retval =
        dataservice_api_sendreq_block_get(
            instance->data, instance->data_child_context,
            instance->previous_block_id);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        instance->data, &canonizationservice_data_write,
        instance->loop_context);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

done:
    return retval;
}
