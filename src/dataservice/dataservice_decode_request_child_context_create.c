/**
 * \file dataservice/dataservice_decode_request_child_context_create.c
 *
 * \brief Decode a child context create request payload.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bitcap.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

#include "dataservice_protocol_internal.h"

/**
 * \brief Decode a child context create request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param caps          Pointer to the buffer to receive the capabilities for
 *                      this context.  Must be large enough to hold the
 *                      dataservice bit capabilities set.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_child_context_create(
    const void* req, size_t size, void* caps)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != req);
    MODEL_ASSERT(NULL != caps);

    BITCAP(dummy_caps, DATASERVICE_API_CAP_BITS_MAX);

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be equal to the size of the capabilities. */
    if (size != sizeof(dummy_caps))
    {
        return AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* copy the caps. */
    memcpy(caps, breq, size);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
