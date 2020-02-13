/**
 * \file dataservice/dataservice_transaction_promote.c
 *
 * \brief Promote a transaction from the queue by id.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Promote a given transaction by ID from the queue.
 *
 * \param ctx           The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation.
 * \param txn_id        The transaction ID for this transaction.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to call this function.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the transaction uuid could not
 *        be found.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out of memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function could
 *        not create a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read from the database.
 *      - AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE if this function failed to
 *        put to the database.
 */
int dataservice_transaction_promote(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* txn_id)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);
    MODEL_ASSERT(NULL != txn_id);

    /* verify that we are allowed to promote transactions from  the transaction
     * queue. */
    if (!BITCAP_ISSET(child->childcaps,
            DATASERVICE_API_CAP_APP_PQ_TRANSACTION_PROMOTE))
    {
        return AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED;
    }

    return dataservice_transaction_promote_internal(
        child, dtxn_ctx, txn_id);
}

/**
 * \brief Promote a given transaction by ID from the queue.
 *
 * This is the internal version of the function, which does not perform any
 * capabilities checking.  As such, it SHOULD NOT BE USED OUTSIDE OF THE DATA
 * SERVICE.
 *
 * \param ctx           The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation.
 * \param txn_id        The transaction ID for this transaction.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the transaction uuid could not
 *        be found.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out of memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function could
 *        not create a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read from the database.
 *      - AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE if this function failed to
 *        put to the database.
 */
int dataservice_transaction_promote_internal(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* txn_id)
{
    int retval = 0;
    MDB_txn* txn = NULL;
    uint8_t* new_buffer = NULL;
    size_t new_buffer_size = 0U;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);
    MODEL_ASSERT(NULL != txn_id);

    /* verify that this transaction ID is not the begin or end transaction. */
    uint8_t key[16];
    memset(key, 0, sizeof(key));
    int cmp1 = memcmp(txn_id, key, sizeof(key));
    memset(key, 0xff, sizeof(key));
    int cmp2 = memcmp(txn_id, key, sizeof(key));
    if (0 == cmp1 || 0 == cmp2)
    {
        /* these IDs are never found. */
        retval = AGENTD_ERROR_DATASERVICE_NOT_FOUND;
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
        if (0 != mdb_txn_begin(details->env, NULL, 0, &txn))
        {
            retval = AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE;
            goto done;
        }
    }

    /* set the transaction to be used from now on. */
    MDB_txn* update_txn = (NULL != txn) ? txn : parent;

    /* first, query the transaction to get the node data. */
    MDB_val lkey;
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)txn_id;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));
    retval = mdb_get(update_txn, details->pq_db, &lkey, &lval);
    if (MDB_NOTFOUND == retval || lval.mv_size < sizeof(data_transaction_node_t))
    {
        /* the record was not found. */
        retval = AGENTD_ERROR_DATASERVICE_NOT_FOUND;
        goto maybe_transaction_abort;
    }
    else if (0 != retval)
    {
        /* some error has occurred. */
        retval = AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE;
        goto maybe_transaction_abort;
    }

    /* allocate a buffer large enough to hold a copy of this data. */
    new_buffer_size = lval.mv_size;
    new_buffer = (uint8_t*)malloc(new_buffer_size);
    if (NULL == new_buffer)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto maybe_transaction_abort;
    }

    /* copy the node data into the buffer. */
    data_transaction_node_t* node;
    memcpy(new_buffer, lval.mv_data, new_buffer_size);
    node = (data_transaction_node_t*)new_buffer;

    /* update the transaction state. */
    node->net_txn_state = htonl(DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED);

    /* attempt to update the entry. */
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)txn_id;
    lval.mv_size = new_buffer_size;
    lval.mv_data = new_buffer;
    retval = mdb_put(update_txn, details->pq_db, &lkey, &lval, 0);
    if (0 != retval)
    {
        /* some error has occurred. */
        retval = AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE;
        goto cleanup_new_buffer;
    }

    /* commit the transaction if created internally. */
    if (NULL != txn)
    {
        mdb_txn_commit(txn);
        txn = NULL;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

    /* fall-through. */

cleanup_new_buffer:
    memset(new_buffer, 0, new_buffer_size);
    free(new_buffer);

maybe_transaction_abort:
    if (NULL != txn)
    {
        mdb_txn_abort(txn);
    }

done:
    return retval;
}
