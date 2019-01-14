/**
 * \file dataservice/dataservice_block_id_by_height_get.c
 *
 * \brief Get a block ID associated with a given block height.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Get the block ID associated with the given block height.
 *
 * \param child         The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation,
 *                      or NULL.
 * \param height        The block height for this query.
 * \param block_id      Pointer to the block UUID (16 bytes) to set.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if a block was not found for this
 *        block height.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to call this function.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read data from the database.
 *      - AGENTD_ERROR_DATASERVICE_INVALID_INDEX_ENTRY if this function
 *        encountered an invalid index entry.
 */
int dataservice_block_id_by_height_get(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, uint64_t height,
    uint8_t* block_id)
{
    int retval = 0;
    MDB_txn* txn = NULL;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);
    MODEL_ASSERT(NULL != child->root->details);
    MODEL_ASSERT(NULL != block_id);

    /* verify that we are allowed to read the block height database. */
    if (!BITCAP_ISSET(child->childcaps,
            DATASERVICE_API_CAP_APP_BLOCK_ID_BY_HEIGHT_READ))
    {
        retval = AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED;
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
            retval = AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE;
            goto done;
        }
    }

    /* set the transaction to be used for now on. */
    MDB_txn* query_txn = (NULL != txn) ? txn : parent;

    /* query the entry. */
    uint64_t net_height = htonll(height);
    MDB_val lkey;
    lkey.mv_size = sizeof(net_height);
    lkey.mv_data = &net_height;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));

    /* attempt to read this record from the database. */
    retval = mdb_get(query_txn, details->height_db, &lkey, &lval);
    if (MDB_NOTFOUND == retval)
    {
        /* the value was not found. */
        retval = AGENTD_ERROR_DATASERVICE_NOT_FOUND;
        goto maybe_transaction_abort;
    }
    else if (0 != retval)
    {
        /* some error has occurred. */
        retval = AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE;
        goto maybe_transaction_abort;
    }

    /* verify that this value matches what we expect for a uuid. */
    if (lval.mv_size != 16)
    {
        retval = AGENTD_ERROR_DATASERVICE_INVALID_INDEX_ENTRY;
        goto maybe_transaction_abort;
    }

    /* copy the block id. */
    memcpy(block_id, lval.mv_data, 16);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

    /* fall-through. */

maybe_transaction_abort:
    if (NULL != txn)
    {
        mdb_txn_abort(txn);
    }

done:
    return retval;
}
