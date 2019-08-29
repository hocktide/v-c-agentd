/**
 * \file dataservice/dataservice_encode_response_child_context_create.c
 *
 * \brief Encode the response for a child context create request.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>

#include "dataservice_protocol_internal.h"

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
    void** payload, size_t* payload_size, uint32_t child_offset)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(NULL != payload_size);

    /* allocate memory for the payload. */
    *payload_size = sizeof(uint32_t);
    *payload = malloc(*payload_size);
    if (NULL == *payload)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* encode the child offset in network order. */
    uint32_t net_child_offset = htonl(child_offset);

    /* copy the network order child offset to the payload. */
    memcpy(*payload, &net_child_offset, sizeof(net_child_offset));

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
