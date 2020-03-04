/**
 * \file consensus/consensus_service_dataservice_response_child_context_close.c
 *
 * \brief Handle the response from the data service child context close call.
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
 * \brief Handle the response from the data service child context close call.
 *
 * \param instance      The consensus service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void consensus_service_dataservice_response_child_context_close(
    consensusservice_instance_t* instance, const uint32_t* UNUSED(resp),
    const size_t UNUSED(resp_size))
{
    /* only sleep if we did not max out transactions. */
    bool should_sleep =
        instance->transaction_list->elements != instance->block_max_transactions;

    /* reset the consensus service. */
    consensus_service_reset(instance, should_sleep);
}
