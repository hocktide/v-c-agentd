/**
 * \file dataservice/dataservice_data_txn_abort.c
 *
 * \brief Abort a transaction in the data service.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Abort a transaction.
 *
 * \param txn           The transaction to abort.
 */
void dataservice_data_txn_abort(
    dataservice_transaction_context_t* txn)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != txn);
    MODEL_ASSERT(NULL != txn->child);
    MODEL_ASSERT(NULL != txn->child->root);
    MODEL_ASSERT(NULL != txn->child->root->details);

    /* abort the transaction. */
    mdb_txn_abort(txn->txn);
}
