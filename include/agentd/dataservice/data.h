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
     * \brief The transaction certificate size, in bytes, and in network order.
     */
    uint64_t net_txn_cert_size;

} data_transaction_node_t;

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_API_HEADER_GUARD*/
