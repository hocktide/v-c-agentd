/**
 * \file agentd/authservice/api.h
 *
 * \brief API for the auth service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_AUTHSERVICE_API_HEADER_GUARD
#define AGENTD_AUTHSERVICE_API_HEADER_GUARD

#include <agentd/authservice.h>
#include <agentd/ipc.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus


/**
 * \brief Initialize the auth service.
 * 
 * Initialize by setting the UUID, public and private keys.
 *
 * \param sock          The socket on which this request is made.
 * \param agent_id      The agent's ID (UUID)
 * \param pub_key       The agent's public key
 * \param priv_key      The agent's private key
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if this write operation would block this
 *        thread.
 *      - AGENTD_ERROR_AUTHSERVICE_IPC_WRITE_DATA_FAILURE if an error occurred
 *        when writing to the socket.
 */
int auth_service_api_sendreq_initialize(
    ipc_socket_context_t* sock, const uint8_t* agent_id,
    const vccrypt_buffer_t* pub_key, const vccrypt_buffer_t* priv_key);

/**
 * \brief Receive a response from the initialize api method call.
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
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.  Here are a
 * few possible status codes; it is not possible to list them all.
 *      - AGENTD_STATUS_SUCCESS if the remote operation completed successfully.
 *      - AGENTD_ERROR_AUTHSERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet size is invalid.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if the operation was halted because it
 *        would block this thread.
 *      - AGENTD_ERROR_AUTHSERVICE_IPC_READ_DATA_FAILURE if reading data from
 *        the socket failed.
 *      - AGENTD_ERROR_AUTHSERVICE_RECVRESP_UNEXPECTED_DATA_PACKET_SIZE if the
 *        data packet size is unexpected.
 *      - AGENTD_ERROR_AUTHSERVICE_RECVRESP_UNEXPECTED_METHOD_CODE if the
 *        method code was unexpected.
 *      - AGENTD_ERROR_AUTHSERVICE_RECVRESP_MALFORMED_PAYLOAD_DATA if the
 *        payload data was malformed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int auth_service_api_recvresp_initialize(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status);


/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_AUTHSERVICE_API_HEADER_GUARD*/
