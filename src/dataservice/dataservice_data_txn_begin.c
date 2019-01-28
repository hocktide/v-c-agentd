/**
 * \file dataservice/dataservice_data_txn_begin.c
 *
 * \brief Begin a transaction in the data service.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Begin a transaction.
 *
 * On success, this function creates a transaction which must either be
 * committed by calling dataservice_data_txn_commit() or aborted by calling
 * dataservice_data_txn_abort().  The caller is responsible for ensuring that
 * this transaction is committed or aborted either before the parent transaction
 * is committed or aborted or before the data service is destroyed.
 *
 * \param child         The child context under which this transaction should be
 *                      begun.
 * \param txn           The transaction to begin.
 * \param parent        An optional parameter for the parent transaction.  This
 *                      parameter is set to NULL when not used.
 * \param read_only     A flag to indicate whether this transaction is read-only
 *                      (true) or read/write (false).  Note: this flag is
 *                      ignored when creating a child transaction; the parent
 *                      transaction's state overrides this one.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if the transaction
 *        could not begin.
 */
int dataservice_data_txn_begin(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* txn,
    dataservice_transaction_context_t* parent, bool read_only)
{
    int retval;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);
    MODEL_ASSERT(NULL != child->root->details);
    MODEL_ASSERT(NULL != txn);

    /* get the details for this database connection. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)child->root->details;

    /* set the parent transaction. */
    MDB_txn* ptxn = (NULL != parent) ? parent->txn : NULL;

    /* initialize the transaction context structure. */
    memset(txn, 0, sizeof(dataservice_transaction_context_t));
    txn->child = child;

    /* should this transaction be read-only? */
    if (NULL == ptxn && read_only)
    {
        retval = mdb_txn_begin(details->env, NULL, MDB_RDONLY, &txn->txn);
    }
    else
    {
        retval = mdb_txn_begin(details->env, ptxn, 0, &txn->txn);
    }

    /* decode the result of this operation. */
    if (0 != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE;
    }
    else
    {
    }
    retval = AGENTD_STATUS_SUCCESS;

    /* erase the structure if we fail. */
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        memset(txn, 0, sizeof(dataservice_transaction_context_t));
    }

    return retval;
}
