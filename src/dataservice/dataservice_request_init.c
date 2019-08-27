/**
 * \file dataservice/dataservice_request_init.c
 *
 * \brief Initialize a dataservice request structure.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>

#include "dataservice_protocol_internal.h"

/**
 * \brief Initailize a dataservice request structure with a child index.
 *
 * \param breq          The request payload to parse (and increment).
 * \param size          The size of this payload (to decrement).
 * \param dreq          The request structure to initialize.
 * \param dreq_size     Size of the dreq structure.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on sucess.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_request_init(
    const uint8_t** breq, size_t* size, dataservice_request_header_t* dreq,
    size_t dreq_size)
{
    MODEL_ASSERT(NULL != breq);
    MODEL_ASSERT(NULL != *breq);
    MODEL_ASSERT(NULL != size);
    MODEL_ASSERT(NULL != dreq);

    /* start by clearing out this structure. */
    memset(dreq, 0, dreq_size);

    /* set the dispose method and size. */
    dreq->hdr.dispose = &dataservice_request_dispose;
    dreq->size = dreq_size;

    /* the request size must be at least large enough for the child index. */
    if (*size < sizeof(uint32_t))
    {
        return AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* copy the index. */
    uint32_t nchild_index;
    memcpy(&nchild_index, *breq, sizeof(uint32_t));

    /* increment breq and decrement size. */
    *breq += sizeof(uint32_t);
    *size -= sizeof(uint32_t);

    /* decode the index. */
    dreq->child_index = ntohl(nchild_index);

    return AGENTD_STATUS_SUCCESS;
}
