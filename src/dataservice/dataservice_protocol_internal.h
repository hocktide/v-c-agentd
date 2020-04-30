/**
 * \file dataservice/dataservice_protocol_internal.h
 *
 * \brief Internal header for the data service protocol.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_DATASERVICE_PROTOCOL_INTERNAL_HEADER_GUARD
#define AGENTD_DATASERVICE_PROTOCOL_INTERNAL_HEADER_GUARD

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

#include <agentd/bitcap.h>
#include <agentd/dataservice.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <vpr/disposable.h>

/**
 * \brief Payload Request Header.
 */
typedef struct dataservice_request_header
{
    disposable_t hdr;
    size_t size;
    uint32_t child_index;
} dataservice_request_header_t;

/**
 * \brief Payload Artifact Read Request structure.
 */
typedef struct dataservice_request_payload_artifact_read
{
    dataservice_request_header_t hdr;
    uint8_t artifact_id[16];
} dataservice_request_payload_artifact_read_t;

/**
 * \brief Block ID by Height Read Request structure.
 */
typedef struct dataservice_request_block_id_by_height_read
{
    dataservice_request_header_t hdr;
    uint64_t block_height;
} dataservice_request_block_id_by_height_read_t;

/**
 * \brief Block ID Latest Read Request structure.
 */
typedef struct dataservice_request_block_id_latest_read
{
    dataservice_request_header_t hdr;
} dataservice_request_block_id_latest_read_t;

/**
 * \brief Block Make Request structure.
 */
typedef struct dataservice_request_block_make
{
    dataservice_request_header_t hdr;
    uint8_t block_id[16];
    size_t cert_size;
    const uint8_t* cert;
} dataservice_request_block_make_t;

/**
 * \brief Block Read Request structure.
 */
typedef struct dataservice_request_block_read
{
    dataservice_request_header_t hdr;
    uint8_t block_id[16];
    bool read_cert;
} dataservice_request_block_read_t;

/**
 * \brief Canonized Transaction Get Request structure.
 */
typedef struct dataservice_request_canonized_transaction_get
{
    dataservice_request_header_t hdr;
    uint8_t txn_id[16];
    bool read_cert;
} dataservice_request_canonized_transaction_get_t;

/**
 * \brief Child Context Close Request structure.
 */
typedef struct dataservice_request_child_context_close
{
    dataservice_request_header_t hdr;
} dataservice_request_child_context_close_t;

/**
 * \brief Child Context Create Request structure.
 */
typedef struct dataservice_request_child_context_create
{
    dataservice_request_header_t hdr;
    BITCAP(caps, DATASERVICE_API_CAP_BITS_MAX);
} dataservice_request_child_context_create_t;

/**
 * \brief Global Setting Get Request structure.
 */
typedef struct dataservice_request_global_setting_get
{
    dataservice_request_header_t hdr;
    uint64_t key;
} dataservice_request_global_setting_get_t;

/**
 * \brief Global Setting Set Request structure.
 */
typedef struct dataservice_request_global_setting_set
{
    dataservice_request_header_t hdr;
    uint64_t key;
    size_t val_size;
    const void* val;
} dataservice_request_global_setting_set_t;

/**
 * \brief Transaction Drop Request structure.
 */
typedef struct dataservice_request_transaction_drop
{
    dataservice_request_header_t hdr;
    uint8_t txn_id[16];
} dataservice_request_transaction_drop_t;

/**
 * \brief Transaction Promote Request structure.
 */
typedef struct dataservice_request_transaction_promote
{
    dataservice_request_header_t hdr;
    uint8_t txn_id[16];
} dataservice_request_transaction_promote_t;

/**
 * \brief Transaction Get Request structure.
 */
typedef struct dataservice_request_transaction_get
{
    dataservice_request_header_t hdr;
    uint8_t txn_id[16];
} dataservice_request_transaction_get_t;

/**
 * \brief Transaction Get First Request structure.
 */
typedef struct dataservice_request_transaction_get_first
{
    dataservice_request_header_t hdr;
} dataservice_request_transaction_get_first_t;

/**
 * \brief Transaction Submit Request structure.
 */
typedef struct dataservice_request_transaction_submit
{
    dataservice_request_header_t hdr;
    uint8_t txn_id[16];
    uint8_t artifact_id[16];
    size_t cert_size;
    const uint8_t* cert;
} dataservice_request_transaction_submit_t;

/**
 * \brief Initailize a dataservice request structure with a child index.
 *
 * \param breq          The request payload to parse (and increment).
 * \param size          The size of this payload (to decrement).
 * \param dreq          The request structure to initialize.
 * \param dreq_size     Size of the dreq structure.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on sucess.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_request_init(
    const uint8_t** breq, size_t* size, dataservice_request_header_t* dreq,
    size_t dreq_size);

/**
 * \brief Initailize a dataservice request structure without a child context.
 *
 * \param breq          The request payload to parse (and increment).
 * \param size          The size of this payload (to decrement).
 * \param dreq          The request structure to initialize.
 * \param dreq_size     Size of the dreq structure.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on sucess.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_request_init_empty(
    const uint8_t** breq, size_t* size, dataservice_request_header_t* dreq,
    size_t dreq_size);

/**
 * \brief Dispose of a simple dataservice request structure.
 *
 * \param disposable        The disposable to dispose.
 */
void dataservice_request_dispose(void* disposable);

/**
 * \brief Decode an artifact read request into its constituent pieces.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_payload_artifact_read(
    const void* req, size_t size,
    dataservice_request_payload_artifact_read_t* dreq);

/**
 * \brief Encode an artifact read response into a payload packet.
 *
 * \param payload           Pointer to receive the allocated packet payload.
 * \param payload_size      Pointer to receive the size of the payload.
 * \param artifact_id       Pointer to the artifact UUID.               
 * \param txn_first         Pointer to the first transaction UUID.
 * \param txn_latest        Pointer to the latest transaction UUID.
 * \param height_first      The block height of the first transaction.
 * \param height_latest     The block heigth of the latest transaction.
 * \param state_latest      The latest state for this artifact.
 *
 * On successful completion of this function, the payload pointer is updated
 * with a buffer containing the payload packet, and the payload_size pointer is
 * updated with the size of this payload packet.  The caller owns the payload
 * packet and must clear and free it when it is no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 */
int dataservice_encode_response_payload_artifact_read(
    void** payload, size_t* payload_size, const uint8_t* artifact_id,
    const uint8_t* txn_first, const uint8_t* txn_latest, uint64_t height_first,
    uint64_t height_latest, uint32_t state_latest);

/**
 * \brief Decode a read block id by height request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_block_id_by_height_read(
    const void* req, size_t size,
    dataservice_request_block_id_by_height_read_t* dreq);

/**
 * \brief Encode a read block id by height response a payload packet.
 *
 * \param payload           Pointer to receive the allocated packet payload.
 * \param payload_size      Pointer to receive the size of the payload.
 * \param block_id          Pointer to the block UUID.               
 *
 * On successful completion of this function, the payload pointer is updated
 * with a buffer containing the payload packet, and the payload_size pointer is
 * updated with the size of this payload packet.  The caller owns the payload
 * packet and must clear and free it when it is no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 */
int dataservice_encode_response_block_id_by_height_read(
    void** payload, size_t* payload_size, const uint8_t* block_id);

/**
 * \brief Decode a read latest block id request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_block_id_latest_read(
    const void* req, size_t size,
    dataservice_request_block_id_latest_read_t* dreq);

/**
 * \brief Encode a read latest block id response payload packet.
 *
 * \param payload           Pointer to receive the allocated packet payload.
 * \param payload_size      Pointer to receive the size of the payload.
 * \param block_id          Pointer to the block UUID.               
 *
 * On successful completion of this function, the payload pointer is updated
 * with a buffer containing the payload packet, and the payload_size pointer is
 * updated with the size of this payload packet.  The caller owns the payload
 * packet and must clear and free it when it is no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 */
int dataservice_encode_response_block_id_latest_read(
    void** payload, size_t* payload_size, const uint8_t* block_id);

/**
 * \brief Decode a make block request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_block_make(
    const void* req, size_t size, dataservice_request_block_make_t* dreq);

/**
 * \brief Decode a block read request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_block_read(
    const void* req, size_t size, dataservice_request_block_read_t* dreq);

/**
 * \brief Encode a read block read response payload packet.
 *
 * \param payload           Pointer to receive the allocated packet payload.
 * \param payload_size      Pointer to receive the size of the payload.
 * \param block_id          Pointer to the block UUID.               
 * \param prev_id           Pointer to the previous block UUID.               
 * \param next_id           Pointer to the next block UUID.               
 * \param first_txn_id      Pointer to the first transaction UUID.
 * \param block_height      The block height.
 * \param write_cert        Set to true if the block cert should be written.
 * \param cert              Pointer to the block certificate.
 * \param cert_size         Size of the block certificate.
 *
 * On successful completion of this function, the payload pointer is updated
 * with a buffer containing the payload packet, and the payload_size pointer is
 * updated with the size of this payload packet.  The caller owns the payload
 * packet and must clear and free it when it is no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 */
int dataservice_encode_response_block_read(
    void** payload, size_t* payload_size, const uint8_t* block_id,
    const uint8_t* prev_id, const uint8_t* next_id, const uint8_t* first_txn_id,
    uint64_t block_height, bool write_cert, const void* cert, size_t cert_size);

/**
 * \brief Decode a canonized transaction get request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_canonized_transaction_get(
    const void* req, size_t size,
    dataservice_request_canonized_transaction_get_t* dreq);

/**
 * \brief Encode a canonized transaction get response payload packet.
 *
 * \param payload           Pointer to receive the allocated packet payload.
 * \param payload_size      Pointer to receive the size of the payload.
 * \param txn_id            Pointer to the transaction UUID.               
 * \param prev_id           Pointer to the previous transaction UUID.               
 * \param next_id           Pointer to the next transaction UUID.               
 * \param artifact_id       Pointer to the artifact UUID.
 * \param block_id          Pointer to the block UUID.
 * \param net_txn_state     Net transaction state.
 * \param write_cert        Set to true if the txn cert should be written.
 * \param cert              Pointer to the transaction certificate.
 * \param cert_size         Size of the transaction certificate.
 *
 * On successful completion of this function, the payload pointer is updated
 * with a buffer containing the payload packet, and the payload_size pointer is
 * updated with the size of this payload packet.  The caller owns the payload
 * packet and must clear and free it when it is no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 */
int dataservice_encode_response_canonized_transaction_get(
    void** payload, size_t* payload_size, const uint8_t* txn_id,
    const uint8_t* prev_id, const uint8_t* next_id, const uint8_t* artifact_id,
    const uint8_t* block_id, uint32_t net_txn_state, bool write_cert,
    const void* cert, size_t cert_size);

/**
 * \brief Decode a child context close request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_child_context_close(
    const void* req, size_t size,
    dataservice_request_child_context_close_t* dreq);

/**
 * \brief Decode a child context create request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_child_context_create(
    const void* req, size_t size,
    dataservice_request_child_context_create_t* dreq);

/**
 * \brief Encode a child context create response.
 *
 * \param payload           Pointer to receive the allocated packet payload.
 * \param payload_size      Pointer to receive the size of the payload.
 * \param child_offset      The child offset to encode in the response.
 *
 * On successful completion of this function, the payload pointer is updated
 * with a buffer containing the payload packet, and the payload_size pointer is
 * updated with the size of this payload packet.  The caller owns the payload
 * packet and must clear and free it when it is no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 */
int dataservice_encode_response_child_context_create(
    void** payload, size_t* payload_size, uint32_t child_offset);

/**
 * \brief Decode a global setting get request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_global_setting_get(
    const void* req, size_t size,
    dataservice_request_global_setting_get_t* dreq);

/**
 * \brief Decode a global setting set request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \note On successful completion of this call, the dreq->value points to data
 * in req, and should not be freed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_global_setting_set(
    const void* req, size_t size,
    dataservice_request_global_setting_set_t* dreq);

/**
 * \brief Decode a transaction drop request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_transaction_drop(
    const void* req, size_t size, dataservice_request_transaction_drop_t* dreq);

/**
 * \brief Decode a transaction promotion request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_transaction_promote(
    const void* req, size_t size,
    dataservice_request_transaction_promote_t* dreq);

/**
 * \brief Decode a transaction get request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_transaction_get(
    const void* req, size_t size, dataservice_request_transaction_get_t* dreq);

/**
 * \brief Encode a transaction get response payload packet.
 *
 * \param payload           Pointer to receive the allocated packet payload.
 * \param payload_size      Pointer to receive the size of the payload.
 * \param txn_id            Pointer to the transaction UUID.               
 * \param prev_id           Pointer to the previous transaction UUID.               
 * \param next_id           Pointer to the next transaction UUID.               
 * \param artifact_id       Pointer to the artifact UUID.
 * \param net_txn_state     Transaction state.
 * \param cert              Pointer to the transaction certificate.
 * \param cert_size         Size of the transaction certificate.
 *
 * On successful completion of this function, the payload pointer is updated
 * with a buffer containing the payload packet, and the payload_size pointer is
 * updated with the size of this payload packet.  The caller owns the payload
 * packet and must clear and free it when it is no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 */
int dataservice_encode_response_transaction_get(
    void** payload, size_t* payload_size, const uint8_t* txn_id,
    const uint8_t* prev_id, const uint8_t* next_id, const uint8_t* artifact_id,
    uint32_t net_txn_state, const void* cert, size_t cert_size);

/**
 * \brief Decode a transaction get first request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_transaction_get_first(
    const void* req, size_t size,
    dataservice_request_transaction_get_first_t* dreq);

/**
 * \brief Encode a transaction get first response payload packet.
 *
 * \param payload           Pointer to receive the allocated packet payload.
 * \param payload_size      Pointer to receive the size of the payload.
 * \param txn_id            Pointer to the transaction UUID.               
 * \param prev_id           Pointer to the previous transaction UUID.               
 * \param next_id           Pointer to the next transaction UUID.               
 * \param artifact_id       Pointer to the artifact UUID.
 * \param net_txn_state     Transaction state.
 * \param cert              Pointer to the transaction certificate.
 * \param cert_size         Size of the transaction certificate.
 *
 * On successful completion of this function, the payload pointer is updated
 * with a buffer containing the payload packet, and the payload_size pointer is
 * updated with the size of this payload packet.  The caller owns the payload
 * packet and must clear and free it when it is no longer needed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 */
int dataservice_encode_response_transaction_get_first(
    void** payload, size_t* payload_size, const uint8_t* txn_id,
    const uint8_t* prev_id, const uint8_t* next_id, const uint8_t* artifact_id,
    uint32_t net_txn_state, const void* cert, size_t cert_size);

/**
 * \brief Decode a transaction submit request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_transaction_submit(
    const void* req, size_t size,
    dataservice_request_transaction_submit_t* dreq);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_INTERNAL_HEADER_GUARD*/
