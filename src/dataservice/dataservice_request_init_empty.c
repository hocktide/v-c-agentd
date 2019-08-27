/**
 * \file dataservice/dataservice_request_init_empty.c
 *
 * \brief Initialize a dataservice request structure without a child context.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <vpr/parameters.h>

#include "dataservice_protocol_internal.h"

/**
 * \brief Initailize a dataservice request structure without a child context.
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
int dataservice_request_init_empty(
    const uint8_t** UNUSED(breq), size_t* UNUSED(size),
    dataservice_request_header_t* dreq, size_t dreq_size)
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

    /* Set the child index to 0. */
    dreq->child_index = 0U;

    return AGENTD_STATUS_SUCCESS;
}
