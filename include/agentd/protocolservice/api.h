/**
 * \file agentd/protocolservice/api.h
 *
 * \brief API for the protocol service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_PROTOCOLSERVICE_API_HEADER_GUARD
#define AGENTD_PROTOCOLSERVICE_API_HEADER_GUARD

#include <agentd/dataservice/data.h>
#include <agentd/protocolservice.h>
#include <vccrypt/suite.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Request IDs for the unauthorized protocol service.
 */
typedef enum unauthorized_protocol_request_id
{
    UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE = 0x00000000,
    UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_ACKNOWLEDGE = 0x00000001,
    UNAUTH_PROTOCOL_REQ_ID_LATEST_BLOCK_ID_GET = 0x00000002,
    UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_SUBMIT = 0x00000003,
    UNAUTH_PROTOCOL_REQ_ID_BLOCK_BY_ID_GET = 0x00000004,
    UNAUTH_PROTOCOL_REQ_ID_BLOCK_ID_GET_NEXT = 0x00000005,
    UNAUTH_PROTOCOL_REQ_ID_BLOCK_ID_GET_PREV = 0x00000006,
    UNAUTH_PROTOCOL_REQ_ID_BLOCK_ID_BY_HEIGHT_GET = 0x00000007,

    UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_BY_ID_GET = 0x00000010,
    UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_ID_GET_NEXT = 0x00000011,
    UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_ID_GET_PREV = 0x00000012,
    UNAUTH_PROTOCOL_REQ_ID_TRANSACTION_ID_GET_BLOCK_ID = 0x00000013,

    UNAUTH_PROTOCOL_REQ_ID_ARTIFACT_FIRST_TXN_BY_ID_GET = 0x00000020,
    UNAUTH_PROTOCOL_REQ_ID_ARTIFACT_LAST_TXN_BY_ID_GET = 0x00000021,

    UNAUTH_PROTOCOL_REQ_ID_STATUS_GET = 0x0000A000,

    UNAUTH_PROTOCOL_REQ_ID_CLOSE = 0x0000FFFF,
} unauthorized_protocol_request_id_t;

/**
 * \brief Send a handshake request to the API.
 *
 * \param sock              The socket to which this request is written.
 * \param suite             The crypto suite to use for this handshake.
 * \param entity_id         The entity UUID originating this request.
 * \param key_nonce         Buffer to receive the client key nonce for this
 *                          request.  This buffer must not have been previously
 *                          initialized.  On success, this is owned by the
 *                          caller and must be disposed.
 * \param challenge_nonce   Buffer to receive the client challenge nonce for
 *                          this request.  This buffer must not have been
 *                          previously initialized.  On success, this is owned
 *                          by the caller and must be disposed.
 *
 * This function generates entropy data for the nonces based on the suite.  This
 * data is passed to the peer on the other end of the socket.  On a successful
 * return from this function, the key_nonce and challenge_nonce buffers are
 * initialized with this entropy data and must be disposed by the caller.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_handshake_request_block(
    int sock, vccrypt_suite_options_t* suite, const uint8_t* entity_id,
    vccrypt_buffer_t* key_nonce, vccrypt_buffer_t* challenge_nonce);

/**
 * \brief Receive a handshake response from the API.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_id                 The buffer to receive the server's uuid.
 *                                  Must not have been previously initialized.
 * \param client_private_key        The client's private key.
 * \param server_public_key         The buffer to receive the server's public
 *                                  key.  Must not have been previously
 *                                  initialized.
 * \param client_key_nonce          The client key nonce for this handshake.
 * \param client_challenge_nonce    The client challenge nonce for this
 *                                  handshake.
 * \param server_challenge_nonce    The buffer to receive the server's challenge
 *                                  nonce. Must not have been previously
 *                                  initialized.
 * \param shared_secret             The buffer to receive the shared secret on
 *                                  success.  Must not have been previously
 *                                  initialized.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.
 *
 * The handshake is verified, and an error is returned if this verification
 * fails.  On success, the computed shared secret is written to the
 * shared_secret parameter, which is owned by the caller and must be disposed
 * when no longer needed.  Likewise, the server id, server public key, and
 * server challenge nonce buffers are written and owned by the caller, and must
 * be disposed when no longer needed.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_handshake_request_block(
    int sock, vccrypt_suite_options_t* suite,
    vccrypt_buffer_t* server_id,
    const vccrypt_buffer_t* client_private_key,
    vccrypt_buffer_t* server_public_key,
    const vccrypt_buffer_t* client_key_nonce,
    const vccrypt_buffer_t* client_challenge_nonce,
    vccrypt_buffer_t* server_challenge_nonce,
    vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status);

/**
 * \brief Send a handshake acknowledge to the API.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to receive updated client IV.
 * \param shared_secret             The shared secret key for this request.
 * \param server_challenge_nonce    The server challenge nonce for this request.
 *
 * This function sends the handshake acknowledgement as an authorized packet to
 * the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_handshake_ack_block(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret,
    const vccrypt_buffer_t* server_challenge_nonce);

/**
 * \brief Receive a handshake ack response from the API.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to receive the updated server IV.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_handshake_ack_block(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status);

/**
 * \brief Send a Latest Block ID get request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 *
 * This function sends a Latest Block ID get request as an authorized packet to
 * the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_latest_block_id_get_block(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret);

/**
 * \brief Receive a Latest Block ID get response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 * \param block_id                  A pointer to the buffer to receive the
 *                                  latest block ID.  Should NOT BE initialized
 *                                  by the caller / will be valid if both the
 *                                  call succeeds and the remote call to the
 *                                  server succeeds.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * If this call succeeds AND the remote call to the server succeeds, then
 * block_id will be initialized with the the UUID from the server.  This buffer
 * is owned by the caller and must be disposed when no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_latest_block_id_get_block(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status,
    vccrypt_buffer_t* block_id);

/**
 * \brief Send a block id by height get request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 * \param height                    The block height for this request.
 *
 * This function sends a block id by height get request as an authorized packet
 * to the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_block_id_by_height_get_block(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret, uint64_t height);

/**
 * \brief Receive a block id by height get response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 * \param block_id                  A pointer to the buffer to receive the
 *                                  block ID.  Should NOT BE initialized
 *                                  by the caller / will be valid if both the
 *                                  call succeeds and the remote call to the
 *                                  server succeeds.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * If this call succeeds AND the remote call to the server succeeds, then
 * block_id will be initialized with the the UUID from the server.  This buffer
 * is owned by the caller and must be disposed when no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_block_id_by_height_get_block(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status,
    vccrypt_buffer_t* block_id);

/**
 * \brief Send an explicit close connection request to the protocol socket.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 *
 * This function sends a close socket request to the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_close(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret);

/**
 * \brief Receive a response from the explicit close connection request.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 *
 * After the response has been successfully received, this socket can be closed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_close(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret);

/**
 * \brief Send a transaction submission request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 * \param txn_id                    The transaction UUID for this transaction.
 * \param artifact_id               The artifact UUID for this transaction.
 * \param cert                      The buffer holding the raw bytes for this
 *                                  certificate.  Must be a valid crypto buffer.
 *
 * This function sends a transaction submission request to the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_transaction_submit(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret, const uint8_t* txn_id,
    const uint8_t* artifact_id, const vccrypt_buffer_t* cert);

/**
 * \brief Receive a transaction submission request response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * Possible upstream status codes:
 *      - AGENTD_ERROR_PROTOCOLSERVICE_DUPLICATE_TRANSACTION if the transaction
 *        ID has already been seen.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_TRANSACTION_VERIFICATION if an error
 *        occurred verifying the transaction (i.e. bad size, bad attestation,
 *        guard failure, etc.).
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_transaction_submit(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status);

/**
 * \brief Send a block get request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 * \param block_id                  The block UUID to get, or zero_uuid for
 *                                  first block, or 0xff uuid for last block.
 *
 * This function sends a block get request to the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_block_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret, const uint8_t* block_id);

/**
 * \brief Receive a block get response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 * \param block_node                The block node data for this block.
 * \param block_cert                Pointer to be populated with a block
 *                                  certificate on success.  This certificate is
 *                                  dynamically allocated and must be freed by
 *                                  the caller.
 * \param block_cert_size           The size of the block certificate returned.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.  The
 * block certificate will only be populated with a dynamically allocated buffer
 * containing the certificate on success.  The caller is responsible for freeing
 * this buffer.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * Possible upstream status codes:
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the block could not be found.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_block_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status,
    data_block_node_t* block_node, uint8_t** block_cert,
    size_t* block_cert_size);

/**
 * \brief Send a block get next id request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 * \param block_id                  The block UUID to get, or zero_uuid for
 *                                  first block, or 0xff uuid for last block.
 *
 * This function sends a block get request to the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_block_next_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret, const uint8_t* block_id);

/**
 * \brief Receive a block get next id response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 * \param next_id                   Buffer to receive the next id.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.  The
 * next block id buffer will only be populated on success.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * Possible upstream status codes:
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the block could not be found.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_block_next_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status,
    uint8_t* next_id);

/**
 * \brief Send a block get prev id request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 * \param block_id                  The block UUID to get, or zero_uuid for
 *                                  first block, or 0xff uuid for last block.
 *
 * This function sends a block get request to the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_block_prev_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret, const uint8_t* block_id);

/**
 * \brief Receive a block get prev id response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 * \param prev_id                   Buffer to receive the prev id.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.  The
 * prev block id buffer will only be populated on success.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * Possible upstream status codes:
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the block could not be found.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_block_prev_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status,
    uint8_t* prev_id);

/**
 * \brief Send a transaction get request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 * \param txn_id                    The transaction UUID to get.
 *
 * This function sends a transaction get request to the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_transaction_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret, const uint8_t* txn_id);

/**
 * \brief Receive a transaction get response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 * \param txn_node                  The transaction node data for this
 *                                  transaction.
 * \param txn_cert                  Pointer to be populated with a transaction
 *                                  certificate on success.  This certificate is
 *                                  dynamically allocated and must be freed by
 *                                  the caller.
 * \param txn_cert_size             The size of the transaction certificate
 *                                  returned.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.  The
 * transaction certificate will only be populated with a dynamically allocated
 * buffer containing the certificate on success.  The caller is responsible for
 * freeing this buffer.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * Possible upstream status codes:
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the transaction could not be
 *        found.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_transaction_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status,
    data_transaction_node_t* txn_node, uint8_t** txn_cert,
    size_t* txn_cert_size);

/**
 * \brief Send a transaction get next id request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 * \param txn_id                    The transaction UUID to get.
 *
 * This function sends a transaction get request to the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_transaction_next_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret, const uint8_t* txn_id);

/**
 * \brief Receive a transaction get next id response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 * \param next_id                   Buffer to receive the next id.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.  The
 * next block id buffer will only be populated on success.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * Possible upstream status codes:
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the transaction could not be
 *        found.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_transaction_next_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status,
    uint8_t* next_id);

/**
 * \brief Send a transaction get prev id request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 * \param txn_id                    The transaction UUID to get.
 *
 * This function sends a transaction get request to the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_transaction_prev_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret, const uint8_t* txn_id);

/**
 * \brief Receive a transaction get prev id response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 * \param prev_id                   Buffer to receive the prev id.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.  The
 * prev block id buffer will only be populated on success.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * Possible upstream status codes:
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the transaction could not be
 *        found.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_transaction_prev_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status,
    uint8_t* prev_id);

/**
 * \brief Send a transaction get block id request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 * \param txn_id                    The transaction UUID to get.
 *
 * This function sends a transaction get request to the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_transaction_block_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret, const uint8_t* txn_id);

/**
 * \brief Receive a transaction get block id response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 * \param block_id                  Buffer to receive the block id.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.  The
 * prev block id buffer will only be populated on success.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * Possible upstream status codes:
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the transaction could not be
 *        found.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_transaction_block_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status,
    uint8_t* block_id);

/**
 * \brief Send an artifact get first transaction id request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 * \param artifact_id               The artifact UUID to get.
 *
 * This function sends an artifact get first transaction id request to the
 * server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_artifact_first_txn_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret, const uint8_t* artifact_id);

/**
 * \brief Receive an artifact get first transaction id response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 * \param first_txn_id              Buffer to receive the first txn id.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.  The
 * first txn id buffer will only be populated on success.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * Possible upstream status codes:
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the transaction could not be
 *        found.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_artifact_first_txn_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status,
    uint8_t* first_txn_id);

/**
 * \brief Send an artifact get last transaction id request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 * \param artifact_id               The artifact UUID to get.
 *
 * This function sends an artifact get last transaction id request to the
 * server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_artifact_last_txn_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret, const uint8_t* artifact_id);

/**
 * \brief Receive an artifact get last transaction id response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 * \param last_txn_id               Buffer to receive the last txn id.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.  The
 * first txn id buffer will only be populated on success.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * Possible upstream status codes:
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the transaction could not be
 *        found.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_artifact_last_txn_id_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status,
    uint8_t* last_txn_id);

/**
 * \brief Send a status get request.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to the client IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this request.
 *
 * This function sends a status get request to the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_status_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret);

/**
 * \brief Receive a status get response.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_status_get(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_PROTOCOLSERVICE_API_HEADER_GUARD*/
