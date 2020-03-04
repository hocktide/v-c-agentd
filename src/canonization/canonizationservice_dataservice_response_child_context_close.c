/**
 * \file
 * canonization/canonizationservice_dataservice_response_child_context_close.c
 *
 * \brief Handle the response from the data service child context close call.
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
 * \brief Handle the response from the data service child context close call.
 *
 * \param instance      The canonization service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void canonizationservice_dataservice_response_child_context_close(
    canonizationservice_instance_t* instance, const uint32_t* UNUSED(resp),
    const size_t UNUSED(resp_size))
{
    /* only sleep if we did not max out transactions. */
    bool should_sleep =
        instance->transaction_list->elements != instance->block_max_transactions;

    /* reset the canonization service. */
    canonizationservice_reset(instance, should_sleep);
}
