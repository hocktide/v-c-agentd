/**
 * \file canonization/canonizationservice_dataservice_response_block_read.c
 *
 * \brief Handle the response from the data service block read call.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>

#include "canonizationservice_internal.h"

/**
 * \brief Handle the response from the data service block read.
 *
 * \param instance      The canonization service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void canonizationservice_dataservice_response_block_read(
    canonizationservice_instance_t* instance, const uint32_t* resp,
    const size_t resp_size)
{
    int retval;
    dataservice_response_block_get_t dresp;

    /* decode the response. */
    retval =
        dataservice_decode_response_block_get(
            resp, resp_size, &dresp);
    if (AGENTD_STATUS_SUCCESS != retval || AGENTD_STATUS_SUCCESS != dresp.hdr.status)
    {
        ipc_exit_loop(instance->loop_context);
        goto done;
    }

    /* get the block height. */
    instance->block_height = ntohll(dresp.node.net_block_height) + 1;

    /* get the first transaction in the process queue. */
    retval =
        canonizationservice_dataservice_sendreq_transaction_get_first(
            instance);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        goto done;
    }

done:;
}
