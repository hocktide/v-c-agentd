/**
 * \file dataservice/dataservice_transaction_get_first.c
 *
 * \brief Get the first transaction in the queue.
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
 * \brief Get the first transaction in the queue.
 *
 * \param ctx           The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation.
 * \param node          Optional transaction node details.  If NULL, this is
 *                      ignored.  If not NULL, this structure is provided by the
 *                      caller and is populated by the transaction node data on
 *                      success.
 * \param txn_bytes     Pointer to be updated to the transaction.
 * \param txn_size      Pointer to size to be updated by the size of txn.
 *
 * Note that this transaction will be a COPY if dtxn_ctx is NULL, and a raw
 * pointer to the database data if dtxn_ctx is not NULL which will be valid
 * until the transaction pointed to by dtxn_ctx is committed or released.  If
 * this is a COPY, then the caller is responsible for freeing the memory
 * associated with this copy by calling free().  If this is NOT a COPY, then
 * this memory will be released when dtxn_ctx is committed or released.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if not transactions exist in the
 *        process queue.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this function encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to call this function.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read from the database.
 */
int dataservice_transaction_get_first(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx,
    data_transaction_node_t* node,
    uint8_t** txn_bytes, size_t* txn_size)
{
    int retval = 0;
    MDB_txn* txn = NULL;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);
    MODEL_ASSERT(NULL != txn_bytes);
    MODEL_ASSERT(NULL != txn_size);

    /* verify that we are allowed to read the transaction queue. */
    if (!BITCAP_ISSET(child->childcaps,
            DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ))
    {
        retval = AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED;
        goto done;
    }

    /* get the details for this database connection. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)child->root->details;

    /* set the parent transaction. */
    MDB_txn* parent = (NULL != dtxn_ctx) ? dtxn_ctx->txn : NULL;
    int parent_flags = (NULL != parent) ? 0 : MDB_RDONLY;

    /* create the read transaction for the root transaction node. */
    if (0 != mdb_txn_begin(details->env, parent, parent_flags, &txn))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE;
        txn = NULL;
        goto done;
    }

    /* set up the key and val. */
    uint8_t key[16];
    memset(key, 0, sizeof(key));
    MDB_val lkey;
    lkey.mv_size = sizeof(key);
    lkey.mv_data = key;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));

    /* attempt to read the start of the queue from the database. */
    retval = mdb_get(txn, details->pq_db, &lkey, &lval);
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

    /* verify that this transaction is valid. */
    if (lval.mv_size < sizeof(data_transaction_node_t))
    {
        retval = AGENTD_ERROR_DATASERVICE_INVALID_STORED_TRANSACTION_NODE;
        goto maybe_transaction_abort;
    }

    /* get the node value. */
    data_transaction_node_t* first_node =
        (data_transaction_node_t*)lval.mv_data;

    /* verify that this node's next points to a valid entry. */
    uint8_t last[16];
    memset(last, 0xFF, sizeof(last));
    if (!memcmp(first_node->next, last, sizeof(last)))
    {
        /* this queue is empty. */
        retval = AGENTD_ERROR_DATASERVICE_NOT_FOUND;
        goto maybe_transaction_abort;
    }

    /* copy the new key. */
    memcpy(key, first_node->next, sizeof(key));

    /* stop the transaction (free memory) */
    mdb_txn_abort(txn);
    txn = NULL;

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

    /* set the transaction to be used from now on. */
    MDB_txn* query_txn = (NULL != txn) ? txn : parent;

    /* query the actual first entry. */
    lkey.mv_size = sizeof(key);
    lkey.mv_data = key;
    memset(&lval, 0, sizeof(lval));

    /* attempt to read this node from the database. */
    retval = mdb_get(query_txn, details->pq_db, &lkey, &lval);
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

    /* verify that this value is large enough to be a node value. */
    if (lval.mv_size <= sizeof(data_transaction_node_t))
    {
        retval = AGENTD_ERROR_DATASERVICE_INVALID_STORED_TRANSACTION_NODE;
        goto maybe_transaction_abort;
    }

    /* compute the data size. */
    uint8_t* bdata = (uint8_t*)lval.mv_data;
    size_t data_size =
        ntohll(((data_transaction_node_t*)bdata)->net_txn_cert_size);
    *txn_size = lval.mv_size - sizeof(data_transaction_node_t);

    /* the transaction size should match exactly the data size. */
    if (*txn_size != data_size)
    {
        retval = AGENTD_ERROR_DATASERVICE_INVALID_STORED_TRANSACTION_NODE;
        goto maybe_transaction_abort;
    }

    /* should we copy the data? */
    if (NULL == parent)
    {
        /* alloc the appropriate size for the value. */
        *txn_bytes = (uint8_t*)malloc(*txn_size);
        if (NULL == *txn_bytes)
        {
            retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
            goto maybe_transaction_abort;
        }

        /* copy the bytes. */
        memcpy(
            *txn_bytes, bdata + sizeof(data_transaction_node_t), *txn_size);
    }
    /* pass the data back directly with no copy. */
    else
    {
        *txn_bytes = bdata + sizeof(data_transaction_node_t);
    }

    /* should we populate the node structure? */
    if (NULL != node)
    {
        memcpy(node, bdata, sizeof(data_transaction_node_t));
    }

    /* success on copy. */
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
