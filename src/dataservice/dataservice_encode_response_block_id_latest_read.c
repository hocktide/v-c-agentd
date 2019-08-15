/**
 * \file dataservice/dataservice_encode_response_block_id_latest_read.c
 *
 * \brief Encode the response to the latest block id read request.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>

#include "dataservice_protocol_internal.h"

/**
 * \brief Encode a read latest block id response a payload packet.
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
    void** payload, size_t* payload_size, const uint8_t* block_id)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(NULL != payload_size);
    MODEL_ASSERT(NULL != block_id);

    /* create the payload. */
    *payload_size = 16U;
    *payload = malloc(*payload_size);
    if (NULL == payload)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the block id to the payload. */
    memcpy(*payload, block_id, 16);

    return AGENTD_STATUS_SUCCESS;
}
