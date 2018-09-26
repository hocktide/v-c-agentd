/**
 * \file agentd/dataservice/private/dataservice.h
 *
 * \brief Private internal API for the data service.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
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
 * \returns 0 on success and non-zero on failure.
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
 * \returns 0 on success and non-zero on failure.
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
 * \returns 0 on success and non-zero on failure.
 */
int dataservice_child_context_create(
    dataservice_root_context_t* root, dataservice_child_context_t* child,
    uint32_t* caps);

/**
 * \brief Close a child context.
 *
 * \param child         The child context to close.
 *
 * \returns 0 on success and non-zero on failure.
 */
int dataservice_child_context_close(
    dataservice_child_context_t* child);

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
 * \returns A status code indicating success or failure.
 *          - 0 on success
 *          - 1 if the value is not found.
 *          - 2 if the value would be truncated.
 *          - non-zero on failure.
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
 * \returns A status code indicating success or failure.
 *          - 0 on success
 *          - non-zero on failure.
 */
int dataservice_global_settings_set(
    dataservice_child_context_t* child, uint64_t key, const char* buffer,
    size_t size);

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
 * \returns A status code indicating success or failure.
 *          - 0 on success
 *          - 1 if the transaction queue is empty.
 *          - non-zero on failure.
 */
int dataservice_transaction_get_first(
    dataservice_child_context_t* child,
    dataservice_transaction_context_t* dtxn_ctx,
    transaction_node_t* node,
    uint8_t** txn_bytes, size_t* txn_size);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_PRIVATE_DATASERVICE_HEADER_GUARD*/
