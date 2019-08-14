/**
 * \file dataservice/dataservice_decode_request_payload_artifact_read.c
 *
 * \brief Decode the artifact read request payload.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>

#include "dataservice_protocol_internal.h"

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
    const void* req, size_t size, uint32_t* child_index, uint8_t* artifact_id)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != req);
    MODEL_ASSERT(NULL != child_index);
    MODEL_ASSERT(NULL != artifact_id);

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)req;

    /* the payload size should be equal to the child context plus artifact ID*/
    if (size != (sizeof(uint32_t) + 16))
    {
        return AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* copy the index. */
    uint32_t nchild_index;
    memcpy(&nchild_index, breq, sizeof(uint32_t));

    /* increment breq and decrement size. */
    breq += sizeof(uint32_t);
    size -= sizeof(uint32_t);

    /* decode the index. */
    *child_index = ntohl(nchild_index);

    /* copy the artifact id. */
    memcpy(artifact_id, breq, 16);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
