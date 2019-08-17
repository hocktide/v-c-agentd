/**
 * \file dataservice/dataservice_child_context_lookup.c
 *
 * \brief Look up a child context from an index.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "dataservice_internal.h"

/**
 * \brief Look up a child context from a potentially bad index.
 *
 * \param ctx           Pointer to the child context to populate.
 * \param inst          The data service instance to use for the lookup.
 * \param offset        The offset used for the lookup.
 *
 * This method does bounds checking on an index value and populates a
 * dataservice_child_context_t if valid.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_BAD_INDEX if the index is
 *        invalid.
 *      - AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_INVALID if the index is
 *        invalid.
 */
int dataservice_child_context_lookup(
    dataservice_child_context_t** ctx, dataservice_instance_t* inst,
    uint32_t offset)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(NULL != inst);

    /* check bounds. */
    if (offset >= DATASERVICE_MAX_CHILD_CONTEXTS)
    {
        return AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_BAD_INDEX;
    }

    /* verify that this child context is open. */
    if (NULL == inst->children[offset].hdr.dispose)
    {
        return AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_INVALID;
    }

    /* set the context to the indexed value. */
    *ctx = &inst->children[offset].ctx;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
