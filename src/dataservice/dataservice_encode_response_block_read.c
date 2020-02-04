/**
 * \file dataservice/dataservice_encode_response_block_read.c
 *
 * \brief Encode the response for the block read request.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>

#include "dataservice_protocol_internal.h"

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
    uint64_t block_height, const void* cert, size_t cert_size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(NULL != payload_size);
    MODEL_ASSERT(NULL != block_id);
    MODEL_ASSERT(NULL != prev_id);
    MODEL_ASSERT(NULL != next_id);
    MODEL_ASSERT(NULL != first_txn_id);
    MODEL_ASSERT(NULL != cert);

    /* create the payload. */
    *payload_size = 4 * 16 + sizeof(uint64_t) + cert_size;
    *payload = (uint8_t*)malloc(*payload_size);
    uint8_t* payload_bytes = (uint8_t*)*payload;
    if (NULL == payload_bytes)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* net block height. */
    uint64_t net_block_height = htonll(block_height);

    /* copy the node values to the payload. */
    memcpy(payload_bytes, block_id, 16);
    memcpy(payload_bytes + 16, prev_id, 16);
    memcpy(payload_bytes + 32, next_id, 16);
    memcpy(payload_bytes + 48, first_txn_id, 16);
    memcpy(payload_bytes + 64, &net_block_height, sizeof(net_block_height));

    /* copy the block certificate data to the payload. */
    memcpy(payload_bytes + 72, cert, cert_size);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
