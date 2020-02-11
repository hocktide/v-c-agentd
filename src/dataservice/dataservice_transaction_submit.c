/**
 * \file dataservice/dataservice_transaction_submit.c
 *
 * \brief Submit a transaction to the transaction queue.
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

/* forward decls */
static int dataservice_transaction_submit_create_queue(
    MDB_dbi pq_db, MDB_txn* txn, const uint8_t* txn_id);
static int dataservice_transaction_submit_update_prev(
    MDB_dbi pq_db, MDB_txn* txn, const uint8_t* txn_id, const uint8_t* prev);
static int dataservice_transaction_submit_update_end(
    MDB_dbi pq_db, MDB_txn* txn, const uint8_t* txn_id,
    const data_transaction_node_t* curr_end);

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
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is
 *            not authorized to perform this operation.
 *          - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function
 *            could not begin a database transaction to insert this transaction.
 *          - AGENTD_ERROR_DATASERVICE_INVALID_STORED_TRANSACTION_NODE if this
 *            function encountered an invalid transaction node in the
 *            transaction queue.
 *          - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if a failure occurred
 *            when reading data from the database.
 *          - AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE if a failure occurred
 *            when writing data to the database.
 *          - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition
 *            was detected during this operation.
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
        retval = AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED;
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
        retval = AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE;
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
    retval = mdb_get(txn, details->pq_db, &lkey, &lval);
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
            retval = AGENTD_ERROR_DATASERVICE_INVALID_STORED_TRANSACTION_NODE;
            goto maybe_transaction_abort;
        }
    }
    else if (0 != retval)
    {
        /* some error has occurred. */
        retval = AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE;
        goto maybe_transaction_abort;
    }

    /* create the node we are inserting. */
    size_t newnode_size = sizeof(data_transaction_node_t) + txn_size;
    data_transaction_node_t* newnode =
        (data_transaction_node_t*)malloc(newnode_size);
    if (NULL == newnode)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto maybe_transaction_abort;
    }

    memset(newnode, 0, newnode_size);
    memcpy(((uint8_t*)newnode) + sizeof(data_transaction_node_t),
        txn_bytes, txn_size);
    memcpy(newnode->key, txn_id, sizeof(newnode->key));
    memset(newnode->next, 0xFF, sizeof(newnode->next));
    memcpy(newnode->artifact_id, artifact_id, sizeof(newnode->artifact_id));
    newnode->net_txn_cert_size = htonll(txn_size);
    newnode->net_txn_state =
        htonl(DATASERVICE_TRANSACTION_NODE_STATE_SUBMITTED);

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
    if (0 != mdb_put(txn, details->pq_db, &lkey, &lval, MDB_NOOVERWRITE))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE;
        goto cleanup_newnode;
    }

    /* if the queue does not exist, create start and end. */
    if (!queue_initialized)
    {
        retval = dataservice_transaction_submit_create_queue(
            details->pq_db, txn, txn_id);
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            goto cleanup_newnode;
        }
    }
    /* if the queue DOES exist, update end and end->prev. */
    else
    {
        retval = dataservice_transaction_submit_update_prev(
            details->pq_db, txn, txn_id, end_node->prev);
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            goto cleanup_newnode;
        }

        /* update end_node prev. */
        retval = dataservice_transaction_submit_update_end(
            details->pq_db, txn, txn_id, end_node);
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            goto cleanup_newnode;
        }
    }

    /* commit the transaction. */
    mdb_txn_commit(txn);
    txn = NULL;

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

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
 * \param pq_db         The transaction database handle.
 * \param txn           The transaction under which this queue is created.
 * \param txn_id        The transaction id to use to populate this queue.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE if a failure occurred when
 *        writing data to the database.
 */
static int dataservice_transaction_submit_create_queue(
    MDB_dbi pq_db, MDB_txn* txn, const uint8_t* txn_id)
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
    if (0 != mdb_put(txn, pq_db, &lkey, &lval, 0))
    {
        return AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE;
    }

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    if (0 != mdb_put(txn, pq_db, &lkey, &lval, 0))
    {
        return AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Fetch and update the previous transaction node with the new
 * transaction id.
 *
 * \param pq_db         The transaction database handle.
 * \param txn           The transaction under which this node is updated.
 * \param txn_id        The transaction id to set as prev->next.
 * \param prev          The previous transaction node to update.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if a failure occurred when
 *        reading data from the database.
 *      - AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE if a failure occurred when
 *        writing data to the database.
 */
static int dataservice_transaction_submit_update_prev(
    MDB_dbi pq_db, MDB_txn* txn, const uint8_t* txn_id, const uint8_t* prev)
{
    /* attempt to read prev. */
    MDB_val lkey;
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)prev;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));
    if (0 != mdb_get(txn, pq_db, &lkey, &lval))
    {
        return AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE;
    }

    /* get the node header. */
    data_transaction_node_t* prev_node =
        (data_transaction_node_t*)lval.mv_data;
    size_t prev_size =
        sizeof(data_transaction_node_t) + ntohll(prev_node->net_txn_cert_size);

    /* copy this header into a local value so we can update it. */
    data_transaction_node_t* node = (data_transaction_node_t*)malloc(prev_size);
    if (NULL == node)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the previous node value. */
    memcpy(node, prev_node, prev_size);

    /* update this node header's next to point to the transaction. */
    memcpy(node->next, txn_id, sizeof(node->next));

    /* place this value in the database. */
    lval.mv_size = prev_size;
    lval.mv_data = node;
    int retval = mdb_put(txn, pq_db, &lkey, &lval, 0);

    /* clean up node. */
    memset(node, 0, prev_size);
    free(node);

    /* decode success or failure. */
    if (0 != retval)
        return AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE;
    else
        return AGENTD_STATUS_SUCCESS;
}

/**
 * \brief Update the end transaction node with the new transaction id.
 *
 * \param pq_db         The transaction database handle.
 * \param txn           The transaction under which this node is updated.
 * \param txn_id        The new transaction id to set as end->next.
 * \param curr_end      The current end node, a copy of which is updated.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE if a failure occurred when
 *        writing data to the database.
 */
static int dataservice_transaction_submit_update_end(
    MDB_dbi pq_db, MDB_txn* txn, const uint8_t* txn_id,
    const data_transaction_node_t* curr_end)
{
    /* create a copy of the end node and update it with the current txn id. */
    data_transaction_node_t end;
    memcpy(&end, curr_end, sizeof(end));
    memcpy(end.prev, txn_id, sizeof(end.prev));

    /* update this node. */
    MDB_val lkey;
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    MDB_val lval;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    if (0 != mdb_put(txn, pq_db, &lkey, &lval, 0))
    {
        return AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE;
    }

    /* success */
    return AGENTD_STATUS_SUCCESS;
}
