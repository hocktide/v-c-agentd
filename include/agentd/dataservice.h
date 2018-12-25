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
#include <agentd/bootstrap_config.h>
#include <agentd/config.h>
#include <agentd/dataservice/data.h>
#include <stdbool.h>
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
     * \brief Capability to read an arbitrary transaction from the transaction
     * queue.
     */
    DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ,

    /**
     * \brief Capability to drop a transaction from the queue.
     */
    DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP,

    /**
     * \brief Capability to read an artifact by ID.
     */
    DATASERVICE_API_CAP_APP_ARTIFACT_READ,

    /**
     * \brief Capability to write a block to the block table.
     *
     * This transaction automatically populates the transaction table with
     * transactions in this block and cleans up the process queue of all
     * matching transactions.
     */
    DATASERVICE_API_CAP_APP_BLOCK_WRITE,

    /**
     * \brief Capability to query a block ID by block height.
     */
    DATASERVICE_API_CAP_APP_BLOCK_ID_BY_HEIGHT_READ,

    /**
     * \brief The number of capabilities bits needed for this API.
     *
     * Must be immediately after the last enumerated bit value.
     */
    DATASERVICE_API_CAP_BITS_MAX
};

/**
 * \brief Global settings that can be set or queried.
 */
enum dataservice_global_setting_enum
{
    /**
     * \brief Lower bound of global settings.  Must be the first value in this
     * enumeration.
     */
    DATASERVICE_GLOBAL_SETTING_LOWER_BOUND,

    /**
     * \brief Schema version.  Must proceed directly after lower bound.
     */
    DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION =
        DATASERVICE_GLOBAL_SETTING_LOWER_BOUND,

    /**
     * \brief Upper bound.  Must be the last value in this enumeration.
     */
    DATASERVICE_GLOBAL_SETTING_UPPER_BOUND
};

/**
 * \brief Data service API methods.
 */
enum dataservice_api_method_enum
{
    /**
     * \brief Lower bound of API methods.  Must be the first value in this
     * enumeration.
     */
    DATASERVICE_API_METHOD_LOWER_BOUND,

    /**
     * \brief Create a root context.  Must proceed directly after lower bound.
     */
    DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE =
        DATASERVICE_API_METHOD_LOWER_BOUND,

    /**
     * \brief Further reduce capabilities for this API.
     */
    DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_REDUCE_CAPS,

    /**
     * \brief Create a child context with reduced capabilities from the root
     * context.
     */
    DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE,

    /**
     * \brief Close a child context.
     */
    DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE,

    /**
     * \brief Read a backup of the database.
     */
    DATASERVICE_API_METHOD_LL_DATABASE_BACKUP,

    /**
     * \brief Restore a backup of the database.
     */
    DATASERVICE_API_METHOD_LL_DATABASE_RESTORE,

    /**
     * \brief Upgrade the database schema.
     */
    DATASERVICE_API_METHOD_LL_DATABASE_UPGRADE,

    /**
     * \brief Query a global setting.
     */
    DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_READ,

    /**
     * \brief Set a global setting.
     */
    DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE,

    /**
     * \brief Read the latest block ID.
     */
    DATASERVICE_API_METHOD_APP_BLOCK_ID_LATEST_READ,

    /**
     * \brief Read the next block ID given a block ID.
     */
    DATASERVICE_API_METHOD_APP_BLOCK_ID_NEXT_READ,

    /**
     * \brief Read the previous block ID given a block ID.
     */
    DATASERVICE_API_METHOD_APP_BLOCK_ID_PREV_READ,

    /**
     * \brief Read the block ID of a given transaction by ID.
     */
    DATASERVICE_API_METHOD_APP_BLOCK_ID_WITH_TRANSACTION_READ,

    /**
     * \brief Read a block by ID.
     */
    DATASERVICE_API_METHOD_APP_BLOCK_READ,

    /**
     * \brief Read a transaction by ID.
     */
    DATASERVICE_API_METHOD_APP_TRANSACTION_READ,

    /**
     * \brief Submit a transaction to the process queue.
     */
    DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT,

    /**
     * \brief Read the first transaction from the process queue.
     */
    DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_FIRST_READ,

    /**
     * \brief Read a transaction from the process queue by id.
     */
    DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ,

    /**
     * \brief Drop a transaction from the process queue by id.
     */
    DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_DROP,

    /**
     * \brief Read an artifact by ID.
     */
    DATASERVICE_API_METHOD_APP_ARTIFACT_READ,

    /**
     * \brief Write a block to the block table.
     *
     * This transaction automatically populates the transaction table with
     * transactions in this block and cleans up the process queue of all
     * matching transactions.
     */
    DATASERVICE_API_METHOD_APP_BLOCK_WRITE,

    /**
     * \brief Query a block ID by block height.
     */
    DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ,

    /**
     * \brief The number of methods in this API.
     *
     * Must be immediately after the last enumerated bit value.
     */
    DATASERVICE_API_METHOD_UPPER_BOUND
};

/* forward decl for dataservice_transaction_context. */
struct dataservice_transaction_context;

/**
 * \brief This transaction context structure is used to allow for child database
 * transactions to be created, committed, or aborted.
 */
typedef struct
    dataservice_transaction_context
        dataservice_transaction_context_t;

/**
 * \brief Event loop for the data service.  This is the entry point for the data
 * service.  It handles the details of reacting to events sent over the data
 * service socket.
 *
 * \param datasock      The data service socket.  The data service listens for
 *                      requests on this socket and sends responses.
 * \param logsock       The logging service socket.  The data service logs data
 *                      on this socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - 0 on normal exit.
 *          - non-zero on abnormal exit.
 */
int dataservice_event_loop(int datasock, int logsock);

/**
 * \brief Spawn a data service process using the provided config structure and
 * logger socket.
 *
 * On success, this method sets the file descriptor pointer to the file
 * descriptor for the data service socket.  This can be used by the caller to
 * send requests to the data service and to receive responses from this service.
 * Also, the pointer to the pid for this process is set.  This can be used to
 * signal and wait when this process should be terminated.
 *
 * \param bconf         The bootstrap configuration for this service.
 * \param conf          The configuration for this service.
 * \param logsock       Socket used to communicate with the logger.
 * \param datasock      Pointer to the data service socket, to be updated on
 *                      successful completion of this function.
 * \param datapid       Pointer to the data service pid, to be updated on the
 *                      successful completion of this function.
 * \param runsecure     Set to false if we are not being run in secure mode.
 *
 * \returns 0 on success and non-zero on failure.
 */
int dataservice_proc(
    const bootstrap_config_t* bconf, const agent_config_t* conf, int logsock,
    int* datasock, pid_t* datapid, bool runsecure);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_HEADER_GUARD*/
