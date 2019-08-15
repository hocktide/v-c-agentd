/**
 * \file dataservice/dataservice_decode_request_canonized_transaction_get.c
 *
 * \brief Decode the canonized transaction get request.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>

#include "dataservice_protocol_internal.h"

/**
 * \brief Decode a canonized transaction get request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param child_index   Pointer to receive the child index.
 * \param txn_id        Pointer to a buffer large enough to receive the
 *                      transaction UUID.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_canonized_transaction_get(
    const void* req, size_t size, uint32_t* child_index, uint8_t* txn_id)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != req);
    MODEL_ASSERT(NULL != child_index);
    MODEL_ASSERT(NULL != txn_id);

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be equal to the child context */
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

    /* copy the txn_id. */
    memcpy(txn_id, breq, 16);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
