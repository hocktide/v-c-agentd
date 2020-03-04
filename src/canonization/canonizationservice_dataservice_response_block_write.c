/**
 * \file canonization/canonizationservice_dataservice_response_block_write.c
 *
 * \brief Handle the response from the data service block write call.
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
 * \brief Handle the response from the data service block write.
 *
 * \param instance      The canonization service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void canonizationservice_dataservice_response_block_write(
    canonizationservice_instance_t* instance, const uint32_t* UNUSED(resp),
    const size_t UNUSED(resp_size))
{
    /* close the child context. */
    canonizationservice_child_context_close(instance);
}
