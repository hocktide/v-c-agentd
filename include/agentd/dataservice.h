/**
 * \file agentd/dataservice.h
 *
 * \brief Service level API for the data service.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_DATASERVICE_HEADER_GUARD
#define AGENTD_DATASERVICE_HEADER_GUARD

#include <agentd/bitcap.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Capabilities for the data service.
 */
enum dataservice_api_cap_enum
{
    /**
     * \brief Capability to create a root context.
     */
    DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE,

    /**
     * \brief Capability to further reduce capabilities for this API.
     */
    DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS,

    /**
     * \brief Capability to create a child context with reduced capabilities
     * from the root context.
     */
    DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE,

    /**
     * \brief Capability to close a child context.
     */
    DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE,

    /**
     * \brief Capability to read a backup of the database.
     */
    DATASERVICE_API_CAP_LL_DATABASE_BACKUP,

    /**
     * \brief Capability to restore a backup of the database.
     */
    DATASERVICE_API_CAP_LL_DATABASE_RESTORE,

    /**
     * \brief Capability to upgrade the database schema.
     */
    DATASERVICE_API_CAP_LL_DATABASE_UPGRADE,

    /**
     * \brief Capability to query a global setting.
     */
    DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ,

    /**
     * \brief Capability to set a global setting.
     */
    DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE,

    /**
     * \brief Capability to read the latest block ID.
     */
    DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ,

    /**
     * \brief Capability to read the next block ID given a block ID.
     */
    DATASERVICE_API_CAP_APP_BLOCK_ID_NEXT_READ,

    /**
     * \brief Capability to read the previous block ID given a block ID.
     */
    DATASERVICE_API_CAP_APP_BLOCK_ID_PREV_READ,

    /**
     * \brief Capability to read the block ID of a given transaction by ID.
     */
    DATASERVICE_API_CAP_APP_BLOCK_ID_WITH_TRANSACTION_READ,

    /**
     * \brief Capability to read a block by ID.
     */
    DATASERVICE_API_CAP_APP_BLOCK_READ,

    /**
     * \brief Capability to read a transaction by ID.
     */
    DATASERVICE_API_CAP_APP_TRANSACTION_READ,

    /**
     * \brief Capability to submit a transaction to the process queue.
     */
    DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT,

    /**
     * \brief Capability to read the first transaction from the process queue.
     */
    DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ,

    /**
     * \brief Capability to read the next transaction from the process queue.
     */
    DATASERVICE_API_CAP_APP_PQ_TRANSACTION_NEXT_READ,

    /**
     * \brief Capability to write a block to the block table.
     *
     * This transaction automatically populates the transaction table with
     * transactions in this block and cleans up the process queue of all
     * matching transactions.
     */
    DATASERVICE_API_CAP_APP_BLOCK_WRITE,

    /**
     * \brief The number of capabilities bits needed for this API.
     *
     * Must be immediately after the last enumerated bit value.
     */
    DATASERVICE_API_CAP_BITS_MAX
};

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_HEADER_GUARD*/
