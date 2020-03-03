/**
 * \file consensus/consensusservice_transaction_list_element_dispose.c
 *
 * \brief Disposer for transaction list elements.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/consensusservice.h>
#include <agentd/consensusservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "consensusservice_internal.h"

/**
 * \brief List element disposer for the consensus service transaction linked
 * list.
 *
 * \param alloc_opts    Ignored by this disposer.
 * \param elem          The element to dispose.
 */
void consensusservice_transaction_list_element_dispose(
    allocator_options_t* UNUSED(alloc_opts), void* elem)
{
    consensusservice_transaction_t* txn = (consensusservice_transaction_t*)elem;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != txn);

    /* run the dispose method for this element. */
    dispose((disposable_t*)txn);

    /* free the memory for this transaction. */
    free(txn);
}
