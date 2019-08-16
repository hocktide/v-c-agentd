/**
 * \file dataservice/dataservice_decode_request_global_setting_get.c
 *
 * \brief Decode a global setting get request.
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
 * \brief Decode a global setting get request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param child_index   Pointer to receive the child index.
 * \param key           Pointer to receive the 64-bit key.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_global_setting_get(
    const void* req, size_t size, uint32_t* child_index, uint64_t* key)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != req);
    MODEL_ASSERT(NULL != child_index);
    MODEL_ASSERT(NULL != key);

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be equal to the size of a child context index and
     * the 64-bit global settings key. */
    if (size != sizeof(uint32_t) + sizeof(uint64_t))
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

    /* get the global settings key. */
    uint64_t nkey;
    memcpy(&nkey, breq, sizeof(nkey));

    /* decode the key. */
    *key = ntohll(nkey);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
