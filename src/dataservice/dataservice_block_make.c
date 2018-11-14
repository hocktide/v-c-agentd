/**
 * \file dataservice/dataservice_block_make.c
 *
 * \brief Make a block in the data service.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <cbmc/model_assert.h>
#include <vccert/certificate_types.h>
#include <vccert/fields.h>
#include <vccert/parser.h>
#include <vccrypt/suite.h>
#include <vccrypt/compare.h>
#include <vpr/allocator.h>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/* constants. */
static uint8_t zero_uuid[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t ff_uuid[16] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* forward decls for parser callbacks. */
static bool dummy_txn_resolver(
    void* options, void* parser, const uint8_t* artifact_id,
    const uint8_t* txn_id, vccrypt_buffer_t* output_buffer,
    bool* trusted);
static int32_t dummy_artifact_state_resolver(
    void* options, void* parser, const uint8_t* artifact_id,
    vccrypt_buffer_t* txn_id);
static bool dummy_entity_key_resolver(
    void* options, void* parser, uint64_t height, const uint8_t* entity_id,
    vccrypt_buffer_t* pubenckey_buffer, vccrypt_buffer_t* pubsignkey_buffer);
static vccert_contract_fn_t dummy_contract_resolver(
    void* options, void* parser, const uint8_t* type_id,
    const uint8_t* artifact_id);

/* forward decls */
static int dataservice_block_make_block_uuid_sanity_check(
    const uint8_t* block_id);
static int dataservice_block_make_create_queue(
    MDB_dbi block_db, MDB_txn* txn, const uint8_t* block_id);
static int dataservice_block_make_update_prev(
    MDB_dbi block_db, MDB_txn* txn, const uint8_t* block_id,
    const uint8_t* prev);
static int dataservice_block_make_update_end(
    MDB_dbi block_db, MDB_txn* txn, const uint8_t* block_id,
    const data_block_node_t* curr_end);

/**
 * \brief Make a block in the data service.
 *
 * The data service will scan through a completed block, finding the UUIDs of
 * the transactions associated with the block.  For each UUID, it will
 * automatically remove the transaction from the transaction queue, index the
 * ID, and update its artifact.  This update is done under a single transaction,
 * so all changes either succeed or fail atomically.
 *
 * \param ctx           The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation.
 * \param block_id      The block ID for this block.
 * \param block_data    The block data for this block.
 * \param block_size    The size of this block.
 *
 * \returns A status code indicating success or failure.
 *          - 0 on success
 *          - non-zero on failure.
 */
int dataservice_block_make(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx,
    const uint8_t* block_id,
    const uint8_t* block_data, size_t block_size)
{
    allocator_options_t alloc_opts;
    vccert_parser_options_t parser_options;
    vccert_parser_context_t parser;
    vccrypt_suite_options_t crypto_suite;
    int retval = 0;
    MDB_txn* txn = NULL;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);
    MODEL_ASSERT(NULL != block_id);
    MODEL_ASSERT(NULL != block_data);
    MODEL_ASSERT(block_size > 0);

    /* verify that we are allowed to make a block. */
    if (!BITCAP_ISSET(child->childcaps,
            DATASERVICE_API_CAP_APP_BLOCK_WRITE))
    {
        retval = 3;
        goto done;
    }

    /* get the details for this database connection. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)child->root->details;

    /* create allocator options for this operation. */
    /* TODO - use a pool allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create crypto suite options for this operation. */
    /* TODO - this should occur at startup. */
    if (VCCRYPT_STATUS_SUCCESS != vccrypt_suite_options_init(&crypto_suite, &alloc_opts, VCCRYPT_SUITE_VELO_V1))
    {
        retval = 6;
        goto dispose_alloc_opts;
    }

    /* create parser options for parsing this block. */
    if (VCCERT_STATUS_SUCCESS != vccert_parser_options_init(&parser_options, &alloc_opts, &crypto_suite, &dummy_txn_resolver, &dummy_artifact_state_resolver, &dummy_contract_resolver, &dummy_entity_key_resolver, NULL))
    {
        retval = 7;
        goto dispose_crypto_suite;
    }

    /* create parser for parsing this block. */
    if (VCCERT_STATUS_SUCCESS != vccert_parser_init(&parser_options, &parser, block_data, block_size))
    {
        retval = 8;
        goto dispose_parser_options;
    }

    /* set the parent transaction. */
    MDB_txn* parent = (NULL != dtxn_ctx) ? dtxn_ctx->txn : NULL;

    /* create the transaction under which this operation occurs. */
    if (0 != mdb_txn_begin(details->env, parent, 0, &txn))
    {
        retval = 4;
        txn = NULL;
        goto dispose_parser;
    }

    /* set up the key for querying the latest block. */
    /* the latest block's ID is stored in the database as 
       ffffffff-ffff-ffff-ffff-ffffffffffff */
    uint8_t key[16];
    memset(key, 0xFF, sizeof(key));
    uint64_t expected_block_height = 0;
    uint8_t expected_prev_block_id[16];
    bool block_queue_initialized = false;
    data_block_node_t* end_node = NULL;

    /* query the blockchain for the last block ID. */
    MDB_val lkey;
    lkey.mv_size = sizeof(key);
    lkey.mv_data = key;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));
    retval = mdb_get(txn, details->block_db, &lkey, &lval);
    if (MDB_NOTFOUND == retval)
    {
        /* the block queue needs to be initialized. */
        block_queue_initialized = false;
        /* the previous block ID should be root. */
        memcpy(expected_prev_block_id, vccert_certificate_type_uuid_root_block,
            sizeof(expected_prev_block_id));
        /* the block height should be 1. */
        expected_block_height = 1;
    }
    else if (0 == retval)
    {
        /* the block queue has been initialized. */
        block_queue_initialized = true;
        /* the value is found, so extract data from the block record. */
        end_node = (data_block_node_t*)lval.mv_data;

        /* verify that this block node is valid. */
        if (lval.mv_size < sizeof(data_block_node_t))
        {
            retval = 2;
            goto maybe_transaction_abort;
        }

        /* copy the previous block ID. */
        memcpy(expected_prev_block_id, end_node->prev,
            sizeof(expected_prev_block_id));
        /* get the block height. */
        expected_block_height = ntohll(end_node->net_block_height);
        /* the expected block height should be one greater. */
        ++expected_block_height;
    }
    else if (0 != retval)
    {
        /* some error has occurred. */
        retval = 5;
        goto maybe_transaction_abort;
    }

    /* get the block height. */
    const uint8_t* block_height_raw = NULL;
    size_t block_height_raw_size = 0;
    if (VCCERT_STATUS_SUCCESS != vccert_parser_find_short(&parser, VCCERT_FIELD_TYPE_BLOCK_HEIGHT, &block_height_raw, &block_height_raw_size) || sizeof(uint64_t) != block_height_raw_size)
    {
        retval = 9;
        goto maybe_transaction_abort;
    }

    /* verify the block height of this block last_block->height + 1. */
    uint64_t net_block_height;
    memcpy(&net_block_height, block_height_raw, sizeof(uint64_t));
    if (((uint64_t)ntohll(net_block_height)) != expected_block_height)
    {
        retval = 10;
        goto maybe_transaction_abort;
    }

    /* get the previous block UUID. */
    const uint8_t* block_prev_uuid = NULL;
    size_t block_prev_uuid_size = 0;
    if (VCCERT_STATUS_SUCCESS != vccert_parser_find_short(&parser, VCCERT_FIELD_TYPE_PREVIOUS_BLOCK_UUID, &block_prev_uuid, &block_prev_uuid_size) || 16 != block_prev_uuid_size)
    {
        retval = 11;
        goto maybe_transaction_abort;
    }

    /* verify the previous block ID is last_block->uuid. */
    if (0 != crypto_memcmp(block_prev_uuid, expected_prev_block_id, 16))
    {
        retval = 12;
        goto maybe_transaction_abort;
    }

    /* verify that the block ID is sane. */
    if (0 != dataservice_block_make_block_uuid_sanity_check(block_id))
    {
        retval = 13;
        goto maybe_transaction_abort;
    }

    /* allocate memory for the block node. */
    /* TODO - replace with pool allocation. */
    size_t blocknode_size = sizeof(data_block_node_t) + block_size;
    data_block_node_t* blocknode =
        (data_block_node_t*)malloc(blocknode_size);
    if (NULL == blocknode)
    {
        retval = 14;
        goto maybe_transaction_abort;
    }

    /* populate the block node */
    memset(blocknode, 0, blocknode_size);
    memcpy(((uint8_t*)blocknode) + sizeof(data_block_node_t),
        block_data, block_size);

    /* create the block node. */
    memcpy(blocknode->key, block_id, sizeof(blocknode->key));
    memset(blocknode->next, 0xFF, sizeof(blocknode->next));
    memcpy(blocknode->prev, block_prev_uuid, sizeof(blocknode->prev));
    /* TODO - fill out first txn ID for iterating transactions in block. */
    blocknode->net_block_height = htonll(expected_block_height);
    blocknode->net_block_cert_size = htonll(block_size);

    /* insert the block node. */
    lkey.mv_size = sizeof(blocknode->key);
    lkey.mv_data = blocknode->key;
    lval.mv_size = blocknode_size;
    lval.mv_data = blocknode;
    if (0 != mdb_put(txn, details->block_db, &lkey, &lval, MDB_NOOVERWRITE))
    {
        retval = 15;
        goto cleanup_blocknode;
    }

    /* do we need to initialize the block queue? */
    if (!block_queue_initialized)
    {
        if (0 != dataservice_block_make_create_queue(details->block_db, txn, block_id))
        {
            retval = 16;
            goto cleanup_blocknode;
        }
    }
    /* if the block queue DOES exist, update end and end->prev. */
    else
    {
        /* update the previous block data. */
        if (0 != dataservice_block_make_update_prev(details->block_db, txn, block_id, end_node->prev))
        {
            retval = 17;
            goto cleanup_blocknode;
        }

        /* update the end node's prev. */
        if (0 != dataservice_block_make_update_end(details->block_db, txn, block_id, end_node))
        {
            retval = 18;
            goto cleanup_blocknode;
        }
    }

    /* TODO */
    /* for each wrapped transaction... */
    /* verify that the transaction exists in the queue. */
    /* verify that this transaction ID does not already exist. */
    /* if a create, verify that the artifact ID does not exist. */
    /* if not create, verify that the previous transaction exists. */
    /* verify that the prev trans state matches trans->state. */
    /* verify that artifact ID exists. */
    /* create transaction. */
    /* create / update artifact to include transaction. */
    /* remove transaction from the queue. */

    /* success. */
    retval = 0;

cleanup_blocknode:
    memset(blocknode, 0, blocknode_size);
    free(blocknode);

maybe_transaction_abort:
    if (NULL != txn)
    {
        mdb_txn_abort(txn);
    }

dispose_parser_options:
    dispose((disposable_t*)&parser_options);

dispose_parser:
    dispose((disposable_t*)&parser);

dispose_crypto_suite:
    dispose((disposable_t*)&crypto_suite);

dispose_alloc_opts:
    dispose((disposable_t*)&alloc_opts);

done:
    return retval;
}

/**
 * Dummy transaction resolver.
 */
static bool dummy_txn_resolver(
    void* UNUSED(options), void* UNUSED(parser),
    const uint8_t* UNUSED(artifact_id), const uint8_t* UNUSED(txn_id),
    vccrypt_buffer_t* UNUSED(output_buffer), bool* UNUSED(trusted))
{
    return false;
}

/**
 * Dummy artifact state resolver.
 */
static int32_t dummy_artifact_state_resolver(
    void* UNUSED(options), void* UNUSED(parser),
    const uint8_t* UNUSED(artifact_id), vccrypt_buffer_t* UNUSED(txn_id))
{
    return -1;
}

/**
 * Dummy entity key resolver.
 */
static bool dummy_entity_key_resolver(
    void* UNUSED(options), void* UNUSED(parser), uint64_t UNUSED(height),
    const uint8_t* UNUSED(entity_id),
    vccrypt_buffer_t* UNUSED(pubenckey_buffer),
    vccrypt_buffer_t* UNUSED(pubsignkey_buffer))
{
    return false;
}

/**
 * Dummy contract resolver.
 */
static vccert_contract_fn_t dummy_contract_resolver(
    void* UNUSED(options), void* UNUSED(parser), const uint8_t* UNUSED(type_id),
    const uint8_t* UNUSED(artifact_id))
{
    return NULL;
}

/**
 * \brief Perform a basic sanity check of the block UUID against constants.
 *
 * \param block_id      The block UUID to check.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int dataservice_block_make_block_uuid_sanity_check(
    const uint8_t* block_id)
{
    MODEL_ASSERT(NULL != block_id);

    /* check against root block ID. */
    if (!crypto_memcmp(block_id, vccert_certificate_type_uuid_root_block, 16))
    {
        return 1;
    }

    /* check against all zeroes ID. */
    if (!crypto_memcmp(block_id, zero_uuid, 16))
    {
        return 2;
    }

    /* check against all 0xFFs ID. */
    if (!crypto_memcmp(block_id, ff_uuid, 16))
    {
        return 3;
    }

    /* UUID appears sane. */
    return 0;
}

/**
 * \brief Create the basic blockchain queue in the block database.
 *
 * \param block_db      The block database to update.
 * \param txn           The database transaction under which this update is
 *                      performed.
 * \param block_id      The block ID representing the first block in this queue.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int dataservice_block_make_create_queue(
    MDB_dbi block_db, MDB_txn* txn, const uint8_t* block_id)
{
    MODEL_ASSERT(NULL != txn);
    MODEL_ASSERT(NULL != block_id);

    /* initialise the begin and end block sentinels. */
    data_block_node_t start, end;
    memset(&start, 0, sizeof(start));
    memset(&end, 0, sizeof(end));
    memcpy(start.next, block_id, sizeof(start.next));
    memset(end.key, 0xFF, sizeof(end.key));
    memset(end.next, 0xFF, sizeof(end.next));
    memcpy(end.prev, block_id, sizeof(end.prev));

    /* insert start. */
    MDB_val lkey;
    lkey.mv_size = sizeof(start.key);
    lkey.mv_data = start.key;
    MDB_val lval;
    lval.mv_size = sizeof(start);
    lval.mv_data = &start;
    if (0 != mdb_put(txn, block_db, &lkey, &lval, 0))
    {
        return 1;
    }

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    if (0 != mdb_put(txn, block_db, &lkey, &lval, 0))
    {
        return 2;
    }

    /* success. */
    return 0;
}

/**
 * \brief Update the previous block with the next block ID.
 *
 * \param block_db      The block database to update.
 * \param txn           The database transaction under which this update is
 *                      performed.
 * \param block_id      The block ID representing the next block being insterted
 *                      into the blockchain.
 * \param prev          The uuid of the previous block node to update.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int dataservice_block_make_update_prev(
    MDB_dbi block_db, MDB_txn* txn, const uint8_t* block_id,
    const uint8_t* prev)
{
    /* attempt to read prev. */
    MDB_val lkey;
    lkey.mv_size = 16;
    lkey.mv_data = (uint8_t*)prev;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));
    if (0 != mdb_get(txn, block_db, &lkey, &lval))
    {
        return 1;
    }

    /* get the node header. */
    data_block_node_t* prev_node = (data_block_node_t*)lval.mv_data;
    size_t prev_size =
        sizeof(data_block_node_t) + ntohll(prev_node->net_block_cert_size);

    /* allocate local data so we can update this value. */
    data_block_node_t* node = (data_block_node_t*)malloc(prev_size);
    if (NULL == node)
    {
        return 2;
    }

    /* copy this value to local memory. */
    memcpy(node, prev_node, prev_size);

    /* update this node header's next to point to the new block. */
    memcpy(node->next, block_id, sizeof(node->next));

    /* place this value into the database. */
    lval.mv_size = prev_size;
    lval.mv_data = node;
    int retval = mdb_put(txn, block_db, &lkey, &lval, 0);

    /* clean up node. */
    memset(node, 0, prev_size);
    free(node);

    /* decode success or failure. */
    if (0 != retval)
        return 2;
    else
        return 0;
}

/**
 * \brief Update the end block node with the new block UUID.
 *
 * \param block_db      The block database to update.
 * \param txn           The database transaction under which this update is
 *                      performed.
 * \param block_id      The block ID representing the next block being insterted
 *                      into the blockchain.
 * \param prev          The current end block node to replace.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int dataservice_block_make_update_end(
    MDB_dbi block_db, MDB_txn* txn, const uint8_t* block_id,
    const data_block_node_t* curr_end)
{
    /* create a copy of the end node and update it with the current block id. */
    data_block_node_t end;
    memcpy(&end, curr_end, sizeof(end));
    memcpy(end.prev, block_id, sizeof(end.prev));

    /* update this node. */
    MDB_val lkey;
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    MDB_val lval;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    if (0 != mdb_put(txn, block_db, &lkey, &lval, 0))
    {
        return 1;
    }

    /* success. */
    return 0;
}
