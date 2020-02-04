/**
 * \file dataservice/dataservice_decode_response_memset_disposer.c
 *
 * \brief Clear the response structure.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <cbmc/model_assert.h>

/**
 * \brief The memset disposer simply clears the data structure when disposed.
 *
 * \param disposable    The disposable to clear.
 */
void dataservice_decode_response_memset_disposer(void* disposable)
{
    dataservice_response_header_t* hdr =
        (dataservice_response_header_t*)disposable;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != hdr);

    /* get the size. */
    size_t size = hdr->payload_size;

    /* clear the structure. */
    MODEL_ASSERT(*((const uint8_t*)disposable) >= 0);
    MODEL_ASSERT(*((const uint8_t*)disposable + sizeof(dataservice_response_header_t) + size - 1) >= 0);
    MODEL_EXEMPT(
        memset(disposable, 0, sizeof(dataservice_response_header_t) + size));
}
