/**
 * \file dataservice/dataservice_artifact_get.c
 *
 * \brief Get an artifact from the artifact database.
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
 * \brief Get an artifact record from the data service.
 *
 * \param child         The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation,
 *                      or NULL.
 * \param artifact_id   The artifact ID for this operation.
 * \param record        Artifact record structure to update via this call.
 *
 * \returns A status code indicating success or failure.
 *          - 0 on success.
 *          - non-zero on failure.
 */
int dataservice_artifact_get(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* artifact_id,
    data_artifact_record_t* record)
{
    int retval = 0;
    MDB_txn* txn = NULL;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);
    MODEL_ASSERT(NULL != child->root->details);
    MODEL_ASSERT(NULL != artifact_id);
    MODEL_ASSERT(NULL != record);

    /* verify that we are allowed to read the artifact database. */
    if (!BITCAP_ISSET(child->childcaps,
            DATASERVICE_API_CAP_APP_ARTIFACT_READ))
    {
        retval = 3;
        goto done;
    }

    /* get the details for this database connection. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)child->root->details;

    /* set the parent transaction. */
    MDB_txn* parent = (NULL != dtxn_ctx) ? dtxn_ctx->txn : NULL;

    /* if the parent transaction is NULL, begin a transaction, or else use the
     * parent transaction. */
    if (NULL == parent)
    {
        if (0 != mdb_txn_begin(details->env, NULL, MDB_RDONLY, &txn))
        {
            retval = 4;
            goto done;
        }
    }

    /* set the transaction to be used for now on. */
    MDB_txn* query_txn = (NULL != txn) ? txn : parent;

    /* query the entry. */
    MDB_val lkey;
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)artifact_id;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));

    /* attempt to read this record from the database. */
    retval = mdb_get(query_txn, details->artifact_db, &lkey, &lval);
    if (MDB_NOTFOUND == retval)
    {
        /* the value was not found. */
        retval = 1;
        goto maybe_transaction_abort;
    }
    else if (0 != retval)
    {
        /* some error has occurred. */
        retval = 5;
        goto maybe_transaction_abort;
    }

    /* verify that this value matches what we expect for an artifact record. */
    if (lval.mv_size != sizeof(data_artifact_record_t))
    {
        retval = 2;
        goto maybe_transaction_abort;
    }

    /* populate the record structure. */
    memcpy(record, lval.mv_data, sizeof(data_artifact_record_t));

    /* success. */
    retval = 0;

    /* fall-through. */

maybe_transaction_abort:
    if (NULL != txn)
    {
        mdb_txn_abort(txn);
    }

done:
    return retval;
}
