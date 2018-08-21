/**
 * \file dataservice/dataservice_child_context_close.c
 *
 * \brief Attempt to close a child context.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Close a child context.
 *
 * \param child         The child context to close.
 *
 * \returns 0 on success and non-zero on failure.
 */
int dataservice_child_context_close(
    dataservice_child_context_t* child)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);

    /* verify that we are allowed to close child contexts. */
    if (!BITCAP_ISSET(child->childcaps,
            DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE))
        return 1;

    /* clear out the child context. */
    memset(child, 0, sizeof(dataservice_child_context_t));

    /* success. */
    return 0;
}
