/**
 * \file agentd/dataservice/private/dataservice.h
 *
 * \brief Private internal API for the data service.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_DATASERVICE_PRIVATE_DATASERVICE_HEADER_GUARD
#define AGENTD_DATASERVICE_PRIVATE_DATASERVICE_HEADER_GUARD

#include <agentd/bitcap.h>
#include <agentd/dataservice.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Context structure for the internal data service.
 */
typedef struct dataservice_private_context
{
    /**
     * \brief This structure is disposable.
     */
    disposable_t hdr;

    /**
     * \brief Root capabilities bitset for this context.
     */
    BITCAP(apicaps, DATASERVICE_API_CAP_BITS_MAX);

    /**
     * \brief Opaque pointer to the database details structure.
     */
    void* details;

} dataservice_root_context_t;

/**
 * \brief Child context structure for the data service.
 *
 * This structure is used to further reduce capabilities in the root context for
 * a set of operations.
 */
typedef struct dataservice_child_context
{
    /**
     * \brief the root context for this interface context.
     */
    dataservice_root_context_t* root;

    /**
     * \brief Child context capabilities.
     */
    BITCAP(childcaps, DATASERVICE_API_CAP_BITS_MAX);

} dataservice_child_context_t;

/**
 * \brief Create a root data service context.
 *
 * \param ctx           The private data service context to initialize.
 * \param datadir       The data directory for this private data service.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if the root context is not
 *        authorized to perform this action.
 *      - AGENTD_ERROR_DATASERVICE_MDB_ENV_CREATE_FAILURE if this function
 *        failed to create a database environment.
 *      - AGENTD_ERROR_DATASERVICE_MDB_ENV_SET_MAPSIZE_FAILURE if this function
 *        failed to set the database map size.
 *      - AGENTD_ERROR_DATASERVICE_MDB_ENV_SET_MAXDBS_FAILURE if this function
 *        failed to set the maximum number of databases.
 *      - AGENTD_ERROR_DATASERVICE_MDB_ENV_OPEN_FAILURE if this function failed
 *        to open the database environment.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_DBI_OPEN_FAILURE if this function failed
 *        to open a database instance.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_COMMIT_FAILURE if this function
 *        failed to commit the database open transaction.
 */
int dataservice_root_context_init(
    dataservice_root_context_t* ctx, const char* datadir);

/**
 * \brief Reduce the root capabilities of a private data service instance.
 *
 * \param ctx           The private data service context to modify.
 * \param caps          The capabilities bitset to use for the reduction
 *                      operation.  It is ANDed against the current capabilities
 *                      in the context to create a reduced context.  The data
 *                      structure must be the same size as the capabilities
 *                      structure defined in dataservice_root_context_t.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if the current context lacks
 *        authorization to perform this operation.
 */
int dataservice_root_context_reduce_capabilities(
    dataservice_root_context_t* ctx, uint32_t* caps);

/**
 * \brief Create a child context with further reduced capabilities.
 *
 * \param root          The root context from which the child context inherits
 *                      its capabilities.
 * \param child         The child context to initialize.
 * \param caps          The capabilities bitset to use for the reduction
 *                      operation.  It is ANDed against the root capabilities
 *                      to create a reduced child context.  The data
 *                      structure must be the same size as the capabilities
 *                      structure defined in dataservice_child_context_t.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this context cannot create
 *        child contexts.
 */
int dataservice_child_context_create(
    dataservice_root_context_t* root, dataservice_child_context_t* child,
    uint32_t* caps);

/**
 * \brief Close a child context.
 *
 * \param child         The child context to close.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to close.
 */
int dataservice_child_context_close(
    dataservice_child_context_t* child);

/**
 * \brief Begin a transaction.
 *
 * On success, this function creates a transaction which must either be
 * committed by calling dataservice_data_txn_commit() or aborted by calling
 * dataservice_data_txn_abort().  The caller is responsible for ensuring that
 * this transaction is committed or aborted either before the parent transaction
 * is committed or aborted or before the data service is destroyed.
 *
 * \param child         The child context under which this transaction should be
 *                      begun.
 * \param txn           The transaction to begin.
 * \param parent        An optional parameter for the parent transaction.  This
 *                      parameter is set to NULL when not used.
 * \param read_only     A flag to indicate whether this transaction is read-only
 *                      (true) or read/write (false).  Note: this flag is
 *                      ignored when creating a child transaction; the parent
 *                      transaction's state overrides this one.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if the transaction
 *        could not begin.
 */
int dataservice_data_txn_begin(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* txn,
    dataservice_transaction_context_t* parent, bool read_only);

/**
 * \brief Abort a transaction.
 *
 * \param txn           The transaction to abort.
 */
void dataservice_data_txn_abort(
    dataservice_transaction_context_t* txn);

/**
 * \brief Commit a transaction.
 *
 * \param txn           The transaction to abort.
 */
void dataservice_data_txn_commit(
    dataservice_transaction_context_t* txn);

/**
 * \brief Query a global setting via the dataservice_global_setting_enum
 * enumeration.
 *
 * \param ctx           The child context from which the global setting is
 *                      queried.
 * \param key           The key value to query.
 * \param buffer        The buffer to which the value is written.
 * \param size          The size of the buffer; updated to the size read if
 *                      successful or the size required if a would truncate
 *                      error occurs.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if this global setting could not be
 *        found.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if the child context is not
 *        authorized to call this function.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read from the database.
 *      - AGENTD_ERROR_DATASERVICE_WOULD_TRUNCATE if the provided buffer would
 *        truncate the value.  The size parameter is updated with the size of
 *        this value.
 */
int dataservice_global_settings_get(
    dataservice_child_context_t* child, uint64_t key, char* buffer,
    size_t* size);

/**
 * \brief Set a global setting via the dataservice_global_setting_enum
 * enumeration.
 *
 * \param ctx           The child context from which the global setting is
 *                      set.
 * \param key           The key value to set.
 * \param buffer        The buffer from which the setting is set.
 * \param size          The size of the buffer.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to perform this operation.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this method failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE if this function failed to
 *        write the setting to the database.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_COMMIT_FAILURE if this function
 *        failed to commit a transaction to the database.
 */
int dataservice_global_settings_set(
    dataservice_child_context_t* child, uint64_t key, const char* buffer,
    size_t size);

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
    const uint8_t* artifact_id, const uint8_t* txn_bytes, size_t txn_size);

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
    uint8_t** txn_bytes, size_t* txn_size);

/**
 * \brief Query the queue for a given transaction by UUID.
 *
 * \param ctx           The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation.
 * \param txn_id        The transaction ID for this transaction.
 * \param node          Optional transaction node details.  If NULL, this is
 *                      ignored.  If not NULL, this structure is provided by the
 *                      caller and is populated by the transaction node data on
 *                      success.
 * \param txn_bytes     Pointer to be updated with the transaction.
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
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if this transaction was not found.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this function encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to call this function.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read data from the database.
 */
int dataservice_transaction_get(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* txn_id,
    data_transaction_node_t* node,
    uint8_t** txn_bytes, size_t* txn_size);

/**
 * \brief Drop a given transaction by ID from the queue.
 *
 * \param ctx           The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation.
 * \param txn_id        The transaction ID for this transaction.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to call this function.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the transaction uuid could not
 *        be found.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out of memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function could
 *        not create a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read from the database.
 *      - AGENTD_ERROR_DATASERVICE_MDB_DEL_FAILURE if this function failed to
 *        delete from the database.
 */
int dataservice_transaction_drop(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* txn_id);

/**
 * \brief Promote a given transaction by ID from the queue.
 *
 * \param ctx           The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation.
 * \param txn_id        The transaction ID for this transaction.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to call this function.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the transaction uuid could not
 *        be found.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out of memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function could
 *        not create a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read from the database.
 *      - AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE if this function failed to
 *        put to the database.
 */
int dataservice_transaction_promote(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* txn_id);

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
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if transaction could not be found
 *        in the transaction process queue.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out of memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to call this function.
 *      - AGENTD_ERROR_DATASERVICE_VCCRYPT_SUITE_OPTIONS_INIT_FAILURE if this
 *        function failed to initialize crypto suite options.
 *      - AGENTD_ERROR_DATASERVICE_VCCERT_PARSER_OPTIONS_INIT_FAILURE if this
 *        function failed to initialize parser options.
 *      - AGENTD_ERROR_DATASERVICE_VCCERT_PARSER_INIT_FAILURE if this
 *        function failed to initialize a parser.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to create a database transaction.
 *      - AGENTD_ERROR_DATASERVICE_INVALID_STORED_BLOCK_NODE if this function
 *        encountered an invalid block node in the database.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function could not
 *        read from the database.
 *      - AGENTD_ERROR_DATASERVICE_MDB_PUT_FAILURE if this function failed to
 *        update the database.
 *      - AGENTD_ERROR_DATASERVICE_MDB_DEL_FAILURE if this function failed to
 *        delete from the database.
 *      - AGENTD_ERROR_DATASERVICE_MISSING_BLOCK_HEIGHT if this block
 *        certificate is missing a block height field.
 *      - AGENTD_ERROR_DATASERVICE_INVALID_BLOCK_HEIGHT if this block
 *        certificate has a block height that is not valid.
 *      - AGENTD_ERROR_DATASERVICE_MISSING_PREVIOUS_BLOCK_UUID if the previous
 *        block uuid field is missing in this block certificate.
 *      - AGENTD_ERROR_DATASERVICE_INVALID_PREVIOUS_BLOCK_UUID if the previous
 *        block uuid field is invalid or does not match the expected previous
 *        block uuid value.
 *      - AGENTD_ERROR_DATASERVICE_MISSING_BLOCK_UUID if
 *        the block UUID field is missing.
 *      - AGENTD_ERROR_DATASERVICE_INVALID_BLOCK_UUID if the block UUID field is
 *        invalid or violates a constraint.
 *      - AGENTD_ERROR_DATASERVICE_NO_CHILD_TRANSACTIONS if there is not at
 *        least one child transaction in this block.
 *      - AGENTD_ERROR_DATASERVICE_MISSING_CHILD_TRANSACTION_UUID if a child
 *        transaction is missing its transaction UUID.
 *      - AGENTD_ERROR_DATASERVICE_MISSING_CHILD_PREVIOUS_TRANSACTION_UUID if a
 *        child transaction is missing its previous transaction UUID.
 *      - AGENTD_ERROR_DATASERVICE_MISSING_CHILD_ARTIFACT_UUID if a child
 *        transaction is missing its artifact UUID.
 *      - AGENTD_ERROR_DATASERVICE_MISSING_CHILD_STATE if a child transaction is
 *        missing its state field.
 *      - AGENTD_ERROR_DATASERVICE_INVALID_ARTIFACT_NODE_SIZE if an invalid
 *        artifact node was encountered.
 */
int dataservice_block_make(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* block_id,
    const uint8_t* block_data, size_t block_size);

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
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the block was not found.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out of memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to perform this operation.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read data from the database.
 *      - AGENTD_ERROR_DATASERVICE_INVALID_STORED_BLOCK_NODE if the block node
 *        read from the database could not be deserialized.
 */
int dataservice_block_get(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* block_id,
    data_block_node_t* node,
    uint8_t** block_bytes, size_t* block_size);

/**
 * \brief Get the block ID associated with the given block height.
 *
 * \param child         The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation,
 *                      or NULL.
 * \param height        The block height for this query.
 * \param block_id      Pointer to the block UUID (16 bytes) to set.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if a block was not found for this
 *        block height.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to call this function.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read data from the database.
 *      - AGENTD_ERROR_DATASERVICE_INVALID_INDEX_ENTRY if this function
 *        encountered an invalid index entry.
 */
int dataservice_block_id_by_height_get(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, uint64_t height,
    uint8_t* block_id);

/**
 * \brief Get the latest block ID.
 *
 * \param child         The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation,
 *                      or NULL.
 * \param block_id      Pointer to the block UUID (16 bytes) to set.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if no blocks were found.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to call this function.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read data from the database.
 *      - AGENTD_ERROR_DATASERVICE_INVALID_INDEX_ENTRY if this function
 *        encountered an invalid index entry.
 */
int dataservice_latest_block_id_get(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, uint8_t* block_id);

/**
 * \brief Get a block transaction from the data service.
 *
 * \param child         The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation,
 *                      or NULL.
 * \param txn_id        The transaction ID for this operation.
 * \param node          Optional transaction node details.  If NULL, this is
 *                      ignored.  If not NULL, this structure is provided by the
 *                      caller and is populated by the transaction node data on
 *                      success.
 * \param txn_bytes     Pointer to be updated with the transaction.
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
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the transaction was not found.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this function encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to perform this operation.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if a failure occurred when
 *        reading the database.
 *      - AGENTD_ERROR_DATASERVICE_INVALID_STORED_TRANSACTION_NODE if the
 *        transaction node could not be deserialized.
 */
int dataservice_block_transaction_get(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* txn_id,
    data_transaction_node_t* node,
    uint8_t** txn_bytes, size_t* txn_size);

/**
 * \brief Get an artifact record from the data service.
 *
 * \param child         The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation,
 *                      or NULL.
 * \param artifact_id   The artifact ID for this operation.
 * \param record        Artifact record structure to update via this call.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if this artifact could not be
 *        found.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized for this operation.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this operation
 *        failed to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if there was a failure
 *        getting this value.
 *      - AGENTD_ERROR_DATASERVICE_INVALID_ARTIFACT_NODE_SIZE if the artifact
 *        data is an invalid size.
 */
int dataservice_artifact_get(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* artifact_id,
    data_artifact_record_t* record);

/**
 * \brief Query the canonized transaction database for a given transaction by
 *        UUID.
 *
 * \param ctx           The child context for this operation.
 * \param dtxn_ctx      The dataservice transaction context for this operation.
 * \param txn_id        The transaction ID for this transaction.
 * \param node          Optional transaction node details.  If NULL, this is
 *                      ignored.  If not NULL, this structure is provided by the
 *                      caller and is populated by the transaction node data on
 *                      success.
 * \param txn_bytes     Pointer to be updated with the transaction.
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
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if this transaction was not found.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this function encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this child context is not
 *        authorized to call this function.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read data from the database.
 */
int dataservice_canonized_transaction_get(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx, const uint8_t* txn_id,
    data_transaction_node_t* node,
    uint8_t** txn_bytes, size_t* txn_size);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_PRIVATE_DATASERVICE_HEADER_GUARD*/
