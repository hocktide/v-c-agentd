/**
 * \file dataservice/dataservice_root_context_reduce_capabilities.c
 *
 * \brief Reduce the capabilities of the root context.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
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
 * \returns 0 on success and non-zero on failure.
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
        return 1;

    /* reduce the capabilities. */
    BITCAP_INTERSECT(ctx->apicaps, ctx->apicaps, caps);

    /* success. */
    return 0;
}
