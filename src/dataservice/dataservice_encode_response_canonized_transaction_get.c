/**
 * \file dataservice/dataservice_encode_response_canonized_transaction_get.c
 *
 * \brief Encode the canonized transaction get response.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>

#include "dataservice_protocol_internal.h"

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
    const uint8_t* block_id, uint32_t net_txn_state,
    const void* cert, size_t cert_size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(NULL != payload_size);
    MODEL_ASSERT(NULL != txn_id);
    MODEL_ASSERT(NULL != prev_id);
    MODEL_ASSERT(NULL != next_id);
    MODEL_ASSERT(NULL != artifact_id);
    MODEL_ASSERT(NULL != block_id);
    MODEL_ASSERT(NULL != cert);

    /* create the payload. */
    *payload_size = 5 * 16 + 4 + cert_size;
    *payload = (uint8_t*)malloc(*payload_size);
    uint8_t* payload_bytes = (uint8_t*)*payload;
    if (NULL == payload_bytes)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the node values to the payload. */
    memcpy(payload_bytes, txn_id, 16);
    memcpy(payload_bytes + 16, prev_id, 16);
    memcpy(payload_bytes + 32, next_id, 16);
    memcpy(payload_bytes + 48, artifact_id, 16);
    memcpy(payload_bytes + 64, block_id, 16);
    memcpy(payload_bytes + 80, &net_txn_state, sizeof(net_txn_state));

    /* copy the certificate data to the payload. */
    memcpy(payload_bytes + 84, cert, cert_size);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
