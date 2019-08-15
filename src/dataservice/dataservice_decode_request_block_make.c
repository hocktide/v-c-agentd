/**
 * \file dataservice/dataservice_decode_request_block_make.c
 *
 * \brief Decode the block make request.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>

#include "dataservice_protocol_internal.h"

/**
 * \brief Decode a make block request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param child_index   Pointer to receive the child index.
 * \param block_id      Pointer to a buffer large enough to receive the block
 *                      UUID.
 * \param cert          Pointer to receive the start of the block certificate in
 *                      the request payload.  Note that this is a substring in
 *                      the request payload.  It should not be freed.
 * \param cert_size     Pointer to receive the size of the block certificate.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_block_make(
    const void* req, size_t size, uint32_t* child_index, uint8_t* block_id,
    uint8_t** cert, size_t* cert_size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != req);
    MODEL_ASSERT(NULL != child_index);
    MODEL_ASSERT(NULL != block_id);
    MODEL_ASSERT(NULL != cert);
    MODEL_ASSERT(NULL != cert_size);

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be at least the size of the header. */
    if (size < (sizeof(uint32_t) + 16))
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

    /* copy the block id. */
    memcpy(block_id, breq, 16);

    /* increment breq and decrement size. */
    breq += 16;
    size -= 16;

    /* the remaining breq and size are the cert / cert_size. */
    *cert = breq;
    *cert_size = size;

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
