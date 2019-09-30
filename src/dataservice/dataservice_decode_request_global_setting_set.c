/**
 * \file dataservice/dataservice_decode_request_global_setting_set.c
 *
 * \brief Decode a global setting set request.
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
 * \brief Decode a global setting set request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \note On successful completion of this call, the dreq->value points to data
 * in req, and should not be freed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_global_setting_set(
    const void* req, size_t size,
    dataservice_request_global_setting_set_t* dreq)
{
    int retval = AGENTD_STATUS_SUCCESS;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != req);
    MODEL_ASSERT(NULL != dreq);

    /* make working with the request more convenient. */
    const uint8_t* breq = (const uint8_t*)req;

    /* initialize the request structure. */
    retval = dataservice_request_init(&breq, &size, &dreq->hdr, sizeof(*dreq));
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* the remaining payload size should be greater than or equal to the 64-bit
     * global settings key. */
    if (size <= sizeof(dreq->key))
    {
        retval = AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE;
        goto cleanup_dreq;
    }

    /* get the global settings key. */
    uint64_t nkey;
    memcpy(&nkey, breq, sizeof(nkey));

    /* decode the key. */
    dreq->key = ntohll(nkey);

    /* increment breq and decrement size. */
    breq += sizeof(dreq->key);
    size -= sizeof(dreq->key);

    /* save the val / val_size. */
    dreq->val = breq;
    dreq->val_size = size;

    /* success. dreq contents are owned by the caller. */
    goto done;

cleanup_dreq:
    /* we failed, so don't pass dreq contents to the caller. */
    dispose((disposable_t*)dreq);

done:
    return retval;
}
