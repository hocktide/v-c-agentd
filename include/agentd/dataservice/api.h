/**
 * \file agentd/dataservice/private/dataservice.h
 *
 * \brief Internal API for the data service.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_DATASERVICE_API_HEADER_GUARD
#define AGENTD_DATASERVICE_API_HEADER_GUARD

#include <agentd/dataservice.h>
#include <agentd/ipc.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Request the creation of a root data service context.
 *
 * \param sock          The socket on which this request is made.
 * \param datadir       The data directory to open.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_root_context_init_block(
    int sock, const char* datadir);

/**
 * \brief Receive a response from the root context init api method call.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_root_context_init_block(
    int sock, uint32_t* offset, uint32_t* status);

/**
 * \brief Request that the capabilities of the root context be reduced.
 *
 * \param sock          The socket on which this request is made.
 * \param caps          The capabilities to use for the reduction.
 * \param size          The size of the capabilities in bytes.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_root_context_reduce_caps_block(
    int sock, uint32_t* caps, size_t size);

/**
 * \brief Receive a response from the root context reduce capabilities call.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_root_context_reduce_caps_block(
    int sock, uint32_t* offset, uint32_t* status);

/**
 * \brief Request the creation of a root data service context.
 *
 * \param sock          The socket on which this request is made.
 * \param datadir       The data directory to open.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_root_context_init(
    ipc_socket_context_t* sock, const char* datadir);

/**
 * \brief Receive a response from the root context init api method call.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_root_context_init(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status);

/**
 * \brief Request that the capabilities of the root context be reduced.
 *
 * \param sock          The socket on which this request is made.
 * \param caps          The capabilities to use for the reduction.
 * \param size          The size of the capabilities in bytes.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_root_context_reduce_caps(
    ipc_socket_context_t* sock, uint32_t* caps, size_t size);

/**
 * \brief Receive a response from the root context reduce capabilities call.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_root_context_reduce_caps(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status);

/**
 * \brief Create a child context with further reduced capabilities.
 *
 * \param sock          The socket on which this request is made.
 * \param caps          The capabilities to use for this child context.
 * \param size          The size of the capabilities in bytes.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_child_context_create(
    ipc_socket_context_t* sock, uint32_t* caps, size_t size);

/**
 * \brief Receive a response from the child context create API call.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 * \param child         The integer index of the created child context,
 *                      populated if status is 0.
 *
 * On a successful return from this function, the status and child index are
 * updated with the status code from the API request.  This status should be
 * checked.  A zero status indicates success, and a non-zero status indicates
 * failure.  The child index is only good if status is 0.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_child_context_create(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status,
    uint32_t* child);

/**
 * \brief Close the specified child context.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index to be closed.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_child_context_close(
    ipc_socket_context_t* sock, uint32_t child);

/**
 * \brief Receive a response from the child context close API call.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_child_context_close(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status);

/**
 * \brief Query a global setting using the given child context.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index used for the query.
 * \param key           The global key to query.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_global_settings_get(
    ipc_socket_context_t* sock, uint32_t child, uint64_t key);

/**
 * \brief Receive a response from the global settings query.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 * \param data          Pointer to a data buffer to write the data value to.
 * \param data_size     Pointer to the size of the data buffer.  On entry, the
 *                      value this is pointed to is set to the size of the data
 *                      buffer.  On exit, if successful, this size is updated to
 *                      the size written to this buffer.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.  On
 * success, the data value and size are both updated to reflect the data read
 * from the query.
 *
 * A status code of 2 represents a "not found" error code.  In future versions
 * of this API, this will be updated to a enumerated value.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_global_settings_get(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status, void* data,
    size_t* data_size);

/**
 * \brief Set a global setting using a 64-bit key.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index used for this operation.
 * \param key           The global key to set.
 * \param val           Buffer holding the value to set for this key.
 * \param val_size      The size of this key.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_global_settings_set(
    ipc_socket_context_t* sock, uint32_t child, uint64_t key, const void* val,
    uint32_t val_size);

/**
 * \brief Receive a response from the global settings set operation.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.  On
 * success, the data value and size are both updated to reflect the data read
 * from the query.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_global_settings_set(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status);

/**
 * \brief Submit a transaction to the transaction queue.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index used for this operation.
 * \param txn_id        The transaction UUID bytes for this transaction.
 * \param artifact_id   The artifact UUID bytes for this transaction.
 * \param val           Buffer holding the raw bytes for the transaction cert.
 * \param val_size      The size of this transaction cert.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_transaction_submit(
    ipc_socket_context_t* sock, uint32_t child, const uint8_t* txn_id,
    const uint8_t* artifact_id, const void* val, uint32_t val_size);

/**
 * \brief Receive a response from the transaction submit operation.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.  On
 * success, the data value and size are both updated to reflect the data read
 * from the query.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_transaction_submit(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status);

/**
 * \brief Get the first transaction in the transaction queue.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index used for the query.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_transaction_get_first(
    ipc_socket_context_t* sock, uint32_t child);

/**
 * \brief Receive a response from the get first transaction query.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 * \param node          Pointer to the node to be updated with data from this
 *                      node in the queue.
 * \param data          This pointer is updated with the data received from the
 *                      response.  The caller owns this buffer and it must be
 *                      freed when no longer needed.
 * \param data_size     Pointer to the size of the data buffer.  On successful
 *                      execution, this size is updated with the size of the
 *                      data allocated for this buffer.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.  On
 * success, the data pointer and size are both updated to reflect the data read
 * from the query.  This is a dynamically allocated buffer that must be freed by
 * the caller.
 *
 * A status code of 2 represents a "not found" error code.  In future versions
 * of this API, this will be updated to a enumerated value.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_transaction_get_first(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status,
    data_transaction_node_t* node, void** data, size_t* data_size);

/**
 * \brief Get a transaction from the transaction queue by ID.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index used for the query.
 * \param txn_id        The transaction UUID of the transaction to retrieve.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_transaction_get(
    ipc_socket_context_t* sock, uint32_t child, const uint8_t* txn_id);

/**
 * \brief Receive a response from the get transaction query.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 * \param node          Pointer to the node to be updated with data from this
 *                      node in the queue.
 * \param data          This pointer is updated with the data received from the
 *                      response.  The caller owns this buffer and it must be
 *                      freed when no longer needed.
 * \param data_size     Pointer to the size of the data buffer.  On successful
 *                      execution, this size is updated with the size of the
 *                      data allocated for this buffer.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.  On
 * success, the data pointer and size are both updated to reflect the data read
 * from the query.  This is a dynamically allocated buffer that must be freed by
 * the caller.
 *
 * A status code of 1 represents a "not found" error code.  In future versions
 * of this API, this will be updated to a enumerated value.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_transaction_get(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status,
    data_transaction_node_t* node, void** data, size_t* data_size);

/**
 * \brief Drop a transaction from the transaction queue by ID.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index used for the query.
 * \param txn_id        The transaction UUID of the transaction to drop.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_transaction_drop(
    ipc_socket_context_t* sock, uint32_t child, const uint8_t* txn_id);

/**
 * \brief Receive a response from the drop transaction action.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.
 *
 * A status code of 1 represents a "not found" error code.  In future versions
 * of this API, this will be updated to a enumerated value.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_transaction_drop(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status);

/**
 * \brief Get an artifact from the artifact database by ID.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index used for the query.
 * \param artifact_id   The artifact UUID of the artifact to retrieve.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_artifact_get(
    ipc_socket_context_t* sock, uint32_t child, const uint8_t* artifact_id);

/**
 * \brief Receive a response from the get artifact query.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 * \param record        Pointer to the record to be updated with data from this
 *                      artifact record.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.
 *
 * A status code of 1 represents a "not found" error code.  In future versions
 * of this API, this will be updated to a enumerated value.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_artifact_get(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status,
    data_artifact_record_t* record);

/**
 * \brief Make a block from transactions in the transaction queue.
 *
 * Caller submits a valid signed block containing the transactions to drop from
 * the transaction queue.  If this call is successful, then this block and those
 * transactions are canonized.
 *
 * \param sock              The socket on which this request is made.
 * \param child             The child index used for this operation.
 * \param txn_id            The block UUID bytes for this transaction.
 * \param block_cert        Buffer holding the raw bytes for the block cert.
 * \param block_cert_size   The size of this block cert.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_block_make(
    ipc_socket_context_t* sock, uint32_t child, const uint8_t* block_id,
    const void* block_cert, uint32_t block_cert_size);

/**
 * \brief Receive a response from the block make operation.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.  On
 * success, the data value and size are both updated to reflect the data read
 * from the query.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_block_make(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status);

/**
 * \brief Get a block from the dataservice by ID.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index used for the query.
 * \param block_id      The block UUID of the block to retrieve.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_block_get(
    ipc_socket_context_t* sock, uint32_t child, const uint8_t* block_id);

/**
 * \brief Receive a response from the get block query.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 * \param node          Pointer to the node to be updated with data from this
 *                      node in the block database.
 * \param data          This pointer is updated with the data received from the
 *                      response.  The caller owns this buffer and it must be
 *                      freed when no longer needed.
 * \param data_size     Pointer to the size of the data buffer.  On successful
 *                      execution, this size is updated with the size of the
 *                      data allocated for this buffer.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.  On
 * success, the data pointer and size are both updated to reflect the data read
 * from the query.  This is a dynamically allocated buffer that must be freed by
 * the caller.
 *
 * A status code of 1 represents a "not found" error code.  In future versions
 * of this API, this will be updated to a enumerated value.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_block_get(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status,
    data_block_node_t* node, void** data, size_t* data_size);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_API_HEADER_GUARD*/
