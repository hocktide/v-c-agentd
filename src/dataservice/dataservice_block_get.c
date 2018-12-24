/**
 * \file dataservice/dataservice_block_get.c
 *
 * \brief Get a block from the blockchain database by id.
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
 * \brief Query the blockchain for a block by UUID.
 *
 * \param ctx           The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation.
 * \param txn_id        The block ID for this block.
 * \param node          Block node details.  This structure is updated with
 *                      information from the blockchain database.
 * \param block_bytes   Pointer to be updated with the block.
 * \param block_size    Pointer to size to be updated by the size of block.
 *
 * Note that this block will be a COPY if dtxn_ctx is NULL, and a raw
 * pointer to the database data if dtxn_ctx is not NULL which will be valid
 * until the transaction pointed to by dtxn_ctx is committed or released.  If
 * this is a COPY, then the caller is responsible for freeing the memory
 * associated with this copy by calling free().  If this is NOT a COPY, then
 * this memory will be released when dtxn_ctx is committed or released.
 *
 * \returns A status code indicating success or failure.
 *          - 0 on success
 *          - 1 if the block could not be found.
 *          - non-zero on failure.
 */
int dataservice_block_get(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* block_id,
    data_block_node_t* node,
    uint8_t** block_bytes, size_t* block_size)
{
    int retval = 0;
    MDB_txn* txn = NULL;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);
    MODEL_ASSERT(NULL != block_id);
    MODEL_ASSERT(NULL != node);
    MODEL_ASSERT(NULL != block_bytes);
    MODEL_ASSERT(NULL != block_size);

    /* verify that we are allowed to read the transaction queue. */
    if (!BITCAP_ISSET(child->childcaps,
            DATASERVICE_API_CAP_APP_BLOCK_READ))
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

    /* set the transaction to be used from now on. */
    MDB_txn* query_txn = (NULL != txn) ? txn : parent;

    /* query the entry. */
    MDB_val lkey;
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)block_id;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));

    /* attempt to read this node from the database. */
    retval = mdb_get(query_txn, details->block_db, &lkey, &lval);
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

    /* verify that this value is large enough to be a node value. */
    if (lval.mv_size <= sizeof(data_block_node_t))
    {
        retval = 2;
        goto maybe_transaction_abort;
    }

    /* compute the data size. */
    uint8_t* bdata = (uint8_t*)lval.mv_data;
    size_t data_size =
        ntohll(((data_block_node_t*)bdata)->net_block_cert_size);
    *block_size = lval.mv_size - sizeof(data_block_node_t);

    /* the block size should match exactly the data size. */
    if (*block_size != data_size)
    {
        retval = 2;
        goto maybe_transaction_abort;
    }

    /* should we copy the data? */
    if (NULL == parent)
    {
        /* alloc the appropriate size for the value. */
        *block_bytes = (uint8_t*)malloc(*block_size);
        if (NULL == *block_bytes)
        {
            retval = 6;
            goto maybe_transaction_abort;
        }

        /* copy the bytes. */
        memcpy(
            *block_bytes, bdata + sizeof(data_block_node_t), *block_size);
    }
    /* pass the data back directly with no copy. */
    else
    {
        *block_bytes = bdata + sizeof(data_block_node_t);
    }

    /* populate the node structure. */
    memcpy(node, bdata, sizeof(data_block_node_t));

    /* success on copy. */
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
