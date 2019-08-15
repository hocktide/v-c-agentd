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

#include <stdint.h>
#include <stdlib.h>

/**
 * \brief Decode an artifact read request into its constituent pieces.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param child_index   Pointer to receive the child index.
 * \param artifact_id   The buffer to receive the artifact_id.  Must be at least
 *                      16 bytes in size.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_payload_artifact_read(
    const void* req, size_t size, uint32_t* child_index, uint8_t* artifact_id);

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
 * \param child_index   Pointer to receive the child index.
 * \param block_height  The buffer to receive the block_height.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_block_id_by_height_read(
    const void* req, size_t size, uint32_t* child_index,
    uint64_t* block_height);

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

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_INTERNAL_HEADER_GUARD*/
