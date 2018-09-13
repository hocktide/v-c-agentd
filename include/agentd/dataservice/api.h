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

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_API_HEADER_GUARD*/
