/**
 * \file dataservice/dataservice_transaction_submit.c
 *
 * \brief Submit a transaction to the transaction queue.
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
static int dataservice_transaction_submit_create_queue(
    MDB_dbi txn_db, MDB_txn* txn, const uint8_t* txn_id);
static int dataservice_transaction_submit_update_prev(
    MDB_dbi txn_db, MDB_txn* txn, const uint8_t* txn_id, const uint8_t* prev);

/**
 * \brief Submit a transaction to the queue.
 *
 * \param ctx           The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation.
 * \param txn_id        The transaction ID for this transaction.
 * \param artifact_id   The artifact ID for this transaction.
 * \param txn_bytes     The raw bytes of the transaction certificate.
 * \param txn_size      The size of the transaction certificate.
 *
 * \returns A status code indicating success or failure.
 *          - 0 on success
 *          - 1 if the transaction already exists.
 *          - non-zero on failure.
 */
int dataservice_transaction_submit(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* txn_id,
    const uint8_t* artifact_id, const uint8_t* txn_bytes, size_t txn_size)
{
    int retval = 0;
    MDB_txn* txn = NULL;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);
    MODEL_ASSERT(NULL != txn_id);
    MODEL_ASSERT(NULL != artifact_id);
    MODEL_ASSERT(NULL != txn_bytes);
    MODEL_ASSERT(NULL != txn_size);

    /* verify that we are allowed to submit to the transaction queue. */
    if (!BITCAP_ISSET(child->childcaps,
            DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT))
    {
        retval = 3;
        goto done;
    }

    /* get the details for this database connection. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)child->root->details;

    /* set the parent transaction. */
    MDB_txn* parent = (NULL != dtxn_ctx) ? dtxn_ctx->txn : NULL;

    /* create the transaction for the end transaction node. */
    if (0 != mdb_txn_begin(details->env, parent, 0, &txn))
    {
        retval = 4;
        txn = NULL;
        goto done;
    }

    /* set up the key and val. */
    uint8_t key[16];
    memset(key, 0xFF, sizeof(key));
    MDB_val lkey;
    lkey.mv_size = sizeof(key);
    lkey.mv_data = key;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));
    bool queue_initialized = false;
    data_transaction_node_t* end_node = NULL;

    /* attempt to read the end of the queue from the database. */
    retval = mdb_get(txn, details->txn_db, &lkey, &lval);
    if (MDB_NOTFOUND == retval)
    {
        /* the value was not found; we'll need to create it. */
        queue_initialized = false;
        end_node = NULL;
    }
    else if (0 == retval)
    {
        /* the value was found, so the queue is initialized. */
        queue_initialized = true;
        end_node = (data_transaction_node_t*)lval.mv_data;

        /* verify that this transaction is valid. */
        if (lval.mv_size < sizeof(data_transaction_node_t))
        {
            retval = 2;
            goto maybe_transaction_abort;
        }
    }
    else if (0 != retval)
    {
        /* some error has occurred. */
        retval = 5;
        goto maybe_transaction_abort;
    }

    /* create the node we are inserting. */
    size_t newnode_size = sizeof(data_transaction_node_t) + txn_size;
    data_transaction_node_t* newnode =
        (data_transaction_node_t*)malloc(newnode_size);
    memset(newnode, 0, newnode_size);
    memcpy(((uint8_t*)newnode) + sizeof(data_transaction_node_t),
        txn_bytes, txn_size);
    memcpy(newnode->key, txn_id, sizeof(newnode->key));
    memset(newnode->next, 0xFF, sizeof(newnode->next));
    memcpy(newnode->artifact_id, artifact_id, sizeof(newnode->artifact_id));
    newnode->net_txn_cert_size = htonll(txn_size);

    /* if the queue exists, set newnode->prev to the prev of end. */
    if (queue_initialized)
    {
        memcpy(newnode->prev, end_node->prev, sizeof(newnode->prev));
    }
    /* otherwise, prev is start. */
    else
    {
        memset(newnode->prev, 0, sizeof(newnode->prev));
    }

    /* insert the new node. */
    lkey.mv_size = sizeof(newnode->key);
    lkey.mv_data = newnode->key;
    lval.mv_size = newnode_size;
    lval.mv_data = newnode;
    if (0 != mdb_put(txn, details->txn_db, &lkey, &lval, MDB_NOOVERWRITE))
    {
        retval = 6;
        goto cleanup_newnode;
    }

    /* if the queue does not exist, create start and end. */
    if (!queue_initialized)
    {
        if (0 != dataservice_transaction_submit_create_queue(details->txn_db, txn, txn_id))
        {
            retval = 7;
            goto cleanup_newnode;
        }
    }
    /* if the queue DOES exist, update end and end->prev. */
    else
    {
        if (0 != dataservice_transaction_submit_update_prev(details->txn_db, txn, txn_id, end_node->prev))
        {
            retval = 8;
            goto cleanup_newnode;
        }

        /* update end_node prev. */
        /* this can be done directly because we are under a write txn. */
        memcpy(end_node->prev, txn_id, sizeof(end_node->prev));
    }

    /* commit the transaction. */
    mdb_txn_commit(txn);
    txn = NULL;

    /* success. */
    retval = 0;

    /* fall-through. */

cleanup_newnode:
    memset(newnode, 0, newnode_size);
    free(newnode);

maybe_transaction_abort:
    if (NULL != txn)
    {
        mdb_txn_abort(txn);
    }

done:
    return retval;
}

/**
 * \brief Create the submission queue, holding a single element referenced by
 * the given txn_id.
 *
 * \param txn_db        The transaction database handle.
 * \param txn           The transaction under which this queue is created.
 * \param txn_id        The transaction id to use to populate this queue.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int dataservice_transaction_submit_create_queue(
    MDB_dbi txn_db, MDB_txn* txn, const uint8_t* txn_id)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != txn);
    MODEL_ASSERT(NULL != txn_id);

    /* initialize the create and end transaction sentinels. */
    data_transaction_node_t start, end;
    memset(&start, 0, sizeof(start));
    memset(&end, 0, sizeof(end));
    memcpy(start.next, txn_id, sizeof(start.next));
    memset(end.key, 0xFF, sizeof(end.key));
    memset(end.next, 0xFF, sizeof(end.next));
    memcpy(end.prev, txn_id, sizeof(end.prev));

    /* insert start. */
    MDB_val lkey;
    lkey.mv_size = sizeof(start.key);
    lkey.mv_data = start.key;
    MDB_val lval;
    lval.mv_size = sizeof(start);
    lval.mv_data = &start;
    if (0 != mdb_put(txn, txn_db, &lkey, &lval, 0))
    {
        return 1;
    }

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    if (0 != mdb_put(txn, txn_db, &lkey, &lval, 0))
    {
        return 2;
    }

    /* success. */
    return 0;
}

/**
 * \brief Fetch and update the previous transaction node with the new
 * transaction id.
 *
 * \param txn_db        The transaction database handle.
 * \param txn           The transaction under which this node is updated.
 * \param txn_id        The transaction id to set as prev->next.
 * \param prev          The previous transaction node to update.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int dataservice_transaction_submit_update_prev(
    MDB_dbi txn_db, MDB_txn* txn, const uint8_t* txn_id, const uint8_t* prev)
{
    /* attempt to read prev. */
    MDB_val lkey;
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)prev;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));
    if (0 != mdb_get(txn, txn_db, &lkey, &lval))
    {
        return 1;
    }

    /* get the node header. */
    data_transaction_node_t* prev_node =
        (data_transaction_node_t*)lval.mv_data;

    /* update this node header's next to point to the transaction. */
    memcpy(prev_node->next, txn_id, sizeof(prev_node->next));

    /* success. */
    return 0;
}
