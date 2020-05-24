/**
 * \file
 * canonization/canonizationservice_dataservice_response_latest_block_id_read.c
 *
 * \brief Handle the response from the data service latest block id read call.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <vccert/certificate_types.h>

#include "canonizationservice_internal.h"

/**
 * \brief Handle the response from the data service latest block id read.
 *
 * \param instance      The canonization service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void canonizationservice_dataservice_response_latest_block_id_read(
    canonizationservice_instance_t* instance, const uint32_t* resp,
    const size_t resp_size)
{
    int retval;
    dataservice_response_latest_block_id_get_t dresp;

    /* decode the response. */
    retval =
        dataservice_decode_response_latest_block_id_get(
            resp, resp_size, &dresp);
    if (AGENTD_STATUS_SUCCESS != retval || AGENTD_STATUS_SUCCESS != dresp.hdr.status)
    {
        canonizationservice_exit_event_loop(instance);
        goto done;
    }

    /* copy the block id into the previous_block_id. */
    memcpy(instance->previous_block_id, dresp.block_id, 16);

    /* is this the root block? */
    if (!memcmp(
            instance->previous_block_id,
            vccert_certificate_type_uuid_root_block, 16))
    {
        /* the height of the new block is 1. */
        instance->block_height = 1;

        /* get the first transaction in the process queue. */
        retval =
            canonizationservice_dataservice_sendreq_transaction_get_first(
                instance);
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            canonizationservice_exit_event_loop(instance);
            goto done;
        }
    }
    else
    {
        /* read the latest block to get its height. */
        retval =
            canonizationservice_dataservice_sendreq_block_get(
                instance);
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            canonizationservice_exit_event_loop(instance);
            goto done;
        }
    }

done:;
}
