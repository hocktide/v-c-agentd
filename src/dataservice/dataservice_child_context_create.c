/**
 * \file dataservice/dataservice_child_context_create.c
 *
 * \brief Create a child context with reduced capabilities.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Create a child context with further reduced capabilities.
 *
 * \param root          The root context from which the child context inherits
 *                      its capabilities.
 * \param child         The child context to initialize.
 * \param caps          The capabilities bitset to use for the reduction
 *                      operation.  It is ANDed against the root capabilities
 *                      to create a reduced child context.  The data
 *                      structure must be the same size as the capabilities
 *                      structure defined in dataservice_child_context_t.
 *
 * \returns 0 on success and non-zero on failure.
 */
int dataservice_child_context_create(
    dataservice_root_context_t* root, dataservice_child_context_t* child,
    uint32_t* caps)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != root);
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != caps);

    /* verify that we are allowed by root to create child contexts. */
    if (!BITCAP_ISSET(root->apicaps,
            DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE))
        return 1;

    /* verify that we are allowed by the child to create child contexts. */
    /* this check ensures that a child cannot re-create itself. */
    if (!BITCAP_ISSET(child->childcaps,
            DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE))
        return 2;

    /* configure the child context. */
    memset(child, 0, sizeof(dataservice_child_context_t));
    child->root = root;

    /* reduce the child capabilities. */
    BITCAP_INTERSECT(child->childcaps, root->apicaps, caps);

    /* the child cannot create child contexts. */
    /* the child cannot re-create itself. */
    BITCAP_SET_FALSE(
        child->childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* success. */
    return 0;
}
