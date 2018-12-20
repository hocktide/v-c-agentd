/**
 * \file agentd/dataservice/data.h
 *
 * \brief Data types used by the data service.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_DATASERVICE_DATA_HEADER_GUARD
#define AGENTD_DATASERVICE_DATA_HEADER_GUARD

#include <stdint.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief A transaction node is a linked list node backed by the database, which
 * is used to describe a transaction in the transaction queue.
 */
typedef struct data_transaction_node
{
    /**
     * \brief The key for this transaction, i.e. its transaction UUID.
     */
    uint8_t key[16];

    /**
     * \brief The previous transaction ID in the queue.
     */
    uint8_t prev[16];

    /**
     * \brief The next transaction ID in the queue. */
    uint8_t next[16];

    /**
     * \brief The artifact UUID that this transaction describes.
     */
    uint8_t artifact_id[16];

    /**
     * \brief The block to which this transaction belongs, or zeroes for
     * transactions on the queue.
     */
    uint8_t block_id[16];

    /**
     * \brief The transaction certificate size, in bytes, and in network order.
     */
    uint64_t net_txn_cert_size;

} data_transaction_node_t;

/**
 * \brief An artifact record is stores data specific to a transaction in the
 * blockchain database.
 */
typedef struct data_artifact_record
{
    /**
     * \brief The key for this artifact, i.e. its artifact UUID.
     */
    uint8_t key[16];

    /**
     * \brief The first transaction ID describing this artifact.
     */
    uint8_t txn_first[16];

    /**
     * \brief The latest transaction ID describing this artifact.
     */
    uint8_t txn_latest[16];

    /**
     * \brief The block height when this artifact was created, in network order.
     */
    uint64_t net_height_first;

    /**
     * \brief The latest height in which an artifact was updated, in network
     * order.
     */
    uint64_t net_height_latest;

    /**
     * \brief The latest state of the artifact, in network order.
     */
    uint32_t net_state_latest;

} data_artifact_record_t;

/**
 * \brief A block node is a linked list node backed by the database, which is
 * used to describe a block in the blockchain.
 */
typedef struct data_block_node
{
    /**
     * \brief The key for this transaction, i.e. its block UUID.
     */
    uint8_t key[16];

    /**
     * \brief The previous block ID in the blockchain.
     */
    uint8_t prev[16];

    /**
     * \brief The next block ID in the blockchain.
     */
    uint8_t next[16];

    /**
     * \brief The first transaction in the block.
     */
    uint8_t first_transaction_id[16];

    /**
     * \brief The height of this block, in network order.
     */
    uint64_t net_block_height;

    /**
     * \brief The block certificate size, in bytes, and in network order.
     */
    uint64_t net_block_cert_size;

} data_block_node_t;

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_API_HEADER_GUARD*/
