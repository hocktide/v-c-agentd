/**
 * \file dataservice/dataservice_child_context_close.c
 *
 * \brief Attempt to close a child context.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Close a child context.
 *
 * \param child         The child context to close.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to close.
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
        return AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED;

    /* clear out the child context. */
    memset(child, 0, sizeof(dataservice_child_context_t));

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
