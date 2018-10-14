/**
 * \file dataservice/dataservice_transaction_drop.c
 *
 * \brief Drop a transaction from the queue by id.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/* forward decls */
static int dataservice_transaction_drop_fixup_prev_next(
    MDB_txn* del_txn, MDB_dbi txn_db, const data_transaction_node_t* node);

/**
 * \brief Drop a given transaction by ID from the queue.
 *
 * \param ctx           The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation.
 * \param txn_id        The transaction ID for this transaction.
 *
 * \returns A status code indicating success or failure.
 *          - 0 on success
 *          - 1 if the transaction could not be found.
 *          - non-zero on failure.
 */
int dataservice_transaction_drop(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* txn_id)
{
    int retval = 0;
    MDB_txn* txn = NULL;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);
    MODEL_ASSERT(NULL != txn_id);

    /* verify that we are allowed to drop transactions from  the transaction
     * queue. */
    if (!BITCAP_ISSET(child->childcaps,
            DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP))
    {
        retval = 3;
        goto done;
    }

    /* verify that this transaction ID is not the begin or end transaction. */
    uint8_t key[16];
    memset(key, 0, sizeof(key));
    int cmp1 = memcmp(txn_id, key, sizeof(key));
    memset(key, 0xff, sizeof(key));
    int cmp2 = memcmp(txn_id, key, sizeof(key));
    if (0 == cmp1 || 0 == cmp2)
    {
        /* these IDs are never found. */
        retval = 1;
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
            retval = 4;
            goto done;
        }
    }

    /* set the transaction to be used from now on. */
    MDB_txn* del_txn = (NULL != txn) ? txn : parent;

    /* first, query the transaction to get the node data. */
    MDB_val lkey;
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)txn_id;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));
    retval = mdb_get(del_txn, details->txn_db, &lkey, &lval);
    if (MDB_NOTFOUND == retval || lval.mv_size < sizeof(data_transaction_node_t))
    {
        /* the record was not found. */
        retval = 1;
        goto maybe_transaction_abort;
    }
    else if (0 != retval)
    {
        /* some error has occurred. */
        retval = 5;
        goto maybe_transaction_abort;
    }

    /* copy the node data into the node. */
    data_transaction_node_t node;
    memcpy(&node, lval.mv_data, sizeof(node));

    /* attempt to delete the entry. */
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)txn_id;
    retval = mdb_del(del_txn, details->txn_db, &lkey, NULL);
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

    /* update the previous and next records to eliminate this entry. */
    retval =
        dataservice_transaction_drop_fixup_prev_next(
            del_txn, details->txn_db, &node);
    if (0 != retval)
    {
        retval = 6;
        goto maybe_transaction_abort;
    }

    /* commit the transaction if created internally. */
    if (NULL != txn)
    {
        mdb_txn_commit(txn);
        txn = NULL;
    }

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

/**
 * \brief Fix up the next / prev values for the next / prev entries after
 * deleting a transaction.
 *
 * \param del_txn       The transaction used for this delete.
 * \param txn_db        The transaction database.
 * \param node          The node used for getting next and prev.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int dataservice_transaction_drop_fixup_prev_next(
    MDB_txn* del_txn, MDB_dbi txn_db, const data_transaction_node_t* node)
{
    int retval;
    uint8_t* prev_buffer;
    uint8_t* next_buffer;
    size_t prev_buffer_size;
    size_t next_buffer_size;
    data_transaction_node_t* prev;
    data_transaction_node_t* next;

    /* get the previous node. */
    MDB_val lkey;
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)node->prev;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));
    retval = mdb_get(del_txn, txn_db, &lkey, &lval);
    if (0 != retval || lval.mv_size < sizeof(data_transaction_node_t))
    {
        retval = 5;
        goto done;
    }

    /* allocate a buffer large enough to hold a copy of this data. */
    prev_buffer_size = lval.mv_size;
    prev_buffer = (uint8_t*)malloc(prev_buffer_size);
    if (NULL == prev_buffer)
    {
        retval = 6;
        goto done;
    }

    /* update the prev data. */
    memcpy(prev_buffer, lval.mv_data, prev_buffer_size);
    prev = (data_transaction_node_t*)prev_buffer;
    memcpy(prev->next, node->next, sizeof(prev->next));

    /* update the prev record in the database. */
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)node->prev;
    lval.mv_size = prev_buffer_size;
    lval.mv_data = prev_buffer;
    retval = mdb_put(del_txn, txn_db, &lkey, &lval, 0);
    if (0 != retval)
    {
        retval = 7;
        goto maybe_cleanup_prev_buffer;
    }

    /* get the next node. */
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)node->next;
    memset(&lval, 0, sizeof(lval));
    retval = mdb_get(del_txn, txn_db, &lkey, &lval);
    if (0 != retval || lval.mv_size < sizeof(data_transaction_node_t))
    {
        retval = 5;
        goto done;
    }

    /* allocate a buffer large enough to hold a copy of this data. */
    next_buffer_size = lval.mv_size;
    next_buffer = (uint8_t*)malloc(next_buffer_size);
    if (NULL == next_buffer)
    {
        retval = 6;
        goto done;
    }

    /* update the next data. */
    memcpy(next_buffer, lval.mv_data, next_buffer_size);
    next = (data_transaction_node_t*)next_buffer;
    memcpy(next->prev, node->prev, sizeof(next->prev));

    /* update the next record in the database. */
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)node->next;
    lval.mv_size = next_buffer_size;
    lval.mv_data = next_buffer;
    retval = mdb_put(del_txn, txn_db, &lkey, &lval, 0);
    if (0 != retval)
    {
        retval = 7;
        goto maybe_cleanup_next_buffer;
    }

    /* success. */
    retval = 0;

    /* fall-through. */

maybe_cleanup_next_buffer:
    if (NULL != next_buffer)
    {
        memset(next_buffer, 0, next_buffer_size);
        free(next_buffer);
    }

maybe_cleanup_prev_buffer:
    if (NULL != prev_buffer)
    {
        memset(prev_buffer, 0, prev_buffer_size);
        free(prev_buffer);
    }

done:
    return retval;
}
