/**
 * \file dataservice/dataservice_decode_request_child_context_close.c
 *
 * \brief Decode a child context close request.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>

#include "dataservice_protocol_internal.h"

/**
 * \brief Decode a child context close request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param child_index   Pointer to receive the child index.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_child_context_close(
    const void* req, size_t size, uint32_t* child_index)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != req);
    MODEL_ASSERT(NULL != child_index);

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be equal to the size of a child context index. */
    if (size != sizeof(uint32_t))
    {
        return AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* copy the index. */
    uint32_t nchild_index;
    memcpy(&nchild_index, breq, sizeof(uint32_t));

    /* decode the index. */
    *child_index = ntohl(nchild_index);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
