/**
 * \file agentd/randomservice/api.h
 *
 * \brief API for the random service.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_RANDOMSERVICE_API_HEADER_GUARD
#define AGENTD_RANDOMSERVICE_API_HEADER_GUARD

#include <agentd/randomservice.h>
#include <agentd/ipc.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Request some random bytes from the random service (blocking call).
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The offset for the request; will be returned in the
 *                      response.
 * \param count         The number of bytes requested.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_RANDOMSERVICE_IPC_WRITE_DATA_FAILURE if an error occurred
 *        when writing to the socket.
 */
int random_service_api_sendreq_random_bytes_get_block(
    int sock, uint32_t offset, uint32_t count);

/**
 * \brief Receive the response from the random bytes call from the random
 * service (blocking call).
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The offset of the response.
 * \param status        The status of the response.
 * \param bytes         Pointer to receive an allocated buffer of random bytes
 *                      on success.
 * \param bytes_size    The number of bytes received in this buffer on success.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_RANDOMSERVICE_IPC_READ_DATA_FAILURE if an error occurred
 *        when reading from the socket.
 */
int random_service_api_recvresp_random_bytes_get_block(
    int sock, uint32_t* offset, uint32_t* status, void** bytes,
    size_t* bytes_size);

/**
 * \brief Request some random bytes from the random service.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The offset for the request; will be returned in the
 *                      response.
 * \param count         The number of bytes requested.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_RANDOMSERVICE_IPC_WRITE_DATA_FAILURE if an error occurred
 *        when writing to the socket.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if writing this request would cause the
 *        request socket to block.
 */
int random_service_api_sendreq_random_bytes_get(
    ipc_socket_context_t* sock, uint32_t offset, uint32_t count);

/**
 * \brief Receive the response from the random bytes call from the random
 * service.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The offset of the response.
 * \param status        The status of the response.
 * \param bytes         Pointer to receive an allocated buffer of random bytes
 *                      on success.
 * \param bytes_size    The number of bytes received in this buffer on success.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_RANDOMSERVICE_IPC_READ_DATA_FAILURE if an error occurred
 *        when reading from the socket.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if reading this request would cause the
 *        request socket to block.
 */
int random_service_api_recvresp_random_bytes_get(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status,
    void** bytes, size_t* bytes_size);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_RANDOMSERVICE_API_HEADER_GUARD*/
