/**
 * \file dataservice/dataservice_root_context_reduce_capabilities.c
 *
 * \brief Reduce the capabilities of the root context.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Reduce the root capabilities of a private data service instance.
 *
 * \param ctx           The private data service context to modify.
 * \param caps          The capabilities bitset to use for the reduction
 *                      operation.  It is ANDed against the current capabilities
 *                      in the context to create a reduced context.  The data
 *                      structure must be the same size as the capabilities
 *                      structure defined in dataservice_root_context_t.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if the current context lacks
 *        authorization to perform this operation.
 */
int dataservice_root_context_reduce_capabilities(
    dataservice_root_context_t* ctx, uint32_t* caps)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(NULL != caps);

    /* verify that we are allowed to reduce capabilities on the root context. */
    if (!BITCAP_ISSET(ctx->apicaps,
            DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS))
        return AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED;

    /* reduce the capabilities. */
    BITCAP_INTERSECT(ctx->apicaps, ctx->apicaps, caps);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
