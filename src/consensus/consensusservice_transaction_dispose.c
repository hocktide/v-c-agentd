/**
 * \file consensus/consensusservice_transaction_dispose.c
 *
 * \brief Disposer for transaction instances.
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
 * \brief Dispose of a consensusservice_transaction_t instance.
 *
 * \param ptr           Opaque pointer to the transaction instance to be
 *                      disposed.
 */
void consensusservice_transaction_dispose(void* ptr)
{
    consensusservice_transaction_t* txn = (consensusservice_transaction_t*)ptr;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != txn);

    /* clear this value. */
    memset(txn, 0, sizeof(consensusservice_transaction_t) + txn->cert_size);
}
