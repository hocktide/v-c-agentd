/**
 * \file dataservice/dataservice_encode_response_payload_artifact_read.c
 *
 * \brief Encode the response to the artifact read request.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
#include <agentd/status_codes.h>

#include "dataservice_internal.h"
#include "dataservice_protocol_internal.h"

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
    uint64_t height_latest, uint32_t state_latest)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(NULL != payload_size);
    MODEL_ASSERT(NULL != artifact_id);
    MODEL_ASSERT(NULL != txn_first);
    MODEL_ASSERT(NULL != txn_latest);

    /* create the payload. */
    *payload_size = 3 * 16 + 2 * sizeof(uint64_t) + sizeof(uint32_t);
    *payload = malloc(*payload_size);
    uint8_t* payload_bytes = (uint8_t*)*payload;
    if (NULL == payload_bytes)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    uint64_t net_height_first = htonll(height_first);
    uint64_t net_height_latest = htonll(height_latest);
    uint32_t net_state_latest = htonll(state_latest);

    /* copy the record values to the payload. */
    memcpy(payload_bytes, artifact_id, 16);
    memcpy(payload_bytes + 16, txn_first, 16);
    memcpy(payload_bytes + 32, txn_latest, 16);
    memcpy(payload_bytes + 48, &net_height_first, sizeof(net_height_first));
    memcpy(payload_bytes + 56, &net_height_latest, sizeof(net_height_latest));
    memcpy(payload_bytes + 64, &net_state_latest, sizeof(net_state_latest));

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
