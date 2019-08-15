/**
 * \file dataservice/dataservice_decode_request_block_id_by_height_read.c
 *
 * \brief Decode block id read by height request.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>

#include "dataservice_protocol_internal.h"

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
    uint64_t* block_height)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != req);
    MODEL_ASSERT(NULL != child_index);
    MODEL_ASSERT(NULL != block_height);

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be equal to the child context */
    if (size != (sizeof(uint32_t) + sizeof(uint64_t)))
    {
        return AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* copy the index. */
    uint32_t net_child_index;
    memcpy(&net_child_index, breq, sizeof(net_child_index));

    /* increment breq and decrement size. */
    breq += sizeof(uint32_t);
    size -= sizeof(uint32_t);

    /* decode the index. */
    *child_index = ntohl(net_child_index);

    /* copy the block height. */
    uint64_t net_block_height;
    memcpy(&net_block_height, breq, sizeof(net_block_height));

    /* decode the block height. */
    *block_height = ntohll(net_block_height);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
