/**
 * \file canonization/canonizationservice_transaction_list_element_dispose.c
 *
 * \brief Disposer for transaction list elements.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief List element disposer for the canonization service transaction linked
 * list.
 *
 * \param alloc_opts    Ignored by this disposer.
 * \param elem          The element to dispose.
 */
void canonizationservice_transaction_list_element_dispose(
    allocator_options_t* UNUSED(alloc_opts), void* elem)
{
    canonizationservice_transaction_t* txn =
        (canonizationservice_transaction_t*)elem;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != txn);

    /* run the dispose method for this element. */
    dispose((disposable_t*)txn);

    /* free the memory for this transaction. */
    free(txn);
}
