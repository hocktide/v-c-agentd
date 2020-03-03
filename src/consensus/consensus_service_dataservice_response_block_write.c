/**
 * \file consensus/consensus_service_dataservice_response_block_write.c
 *
 * \brief Handle the response from the data service block write call.
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
 * \brief Handle the response from the data service block write.
 *
 * \param instance      The consensus service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void consensus_service_dataservice_response_block_write(
    consensusservice_instance_t* instance, const uint32_t* UNUSED(resp),
    const size_t UNUSED(resp_size))
{
    /* close the child context. */
    consensus_service_child_context_close(instance);
}
