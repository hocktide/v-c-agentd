/**
 * \file dataservice/dataservice_decode_request_transaction_drop.c
 *
 * \brief Decode a transaction drop request.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>

#include "dataservice_protocol_internal.h"

/**
 * \brief Decode a transaction drop request.
 *
 * \param req           The request payload to parse.
 * \param size          The size of this request payload.
 * \param dreq          The request structure into which this request is
 *                      decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet payload size is incorrect.
 */
int dataservice_decode_request_transaction_drop(
    const void* req, size_t size, dataservice_request_transaction_drop_t* dreq)
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

    /* the remaining payload size should be equal to the UUID size. */
    if (size != sizeof(dreq->txn_id))
    {
        retval = AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE;
        goto cleanup_dreq;
    }

    /* copy the txn_id. */
    memcpy(dreq->txn_id, breq, 16);

    /* success. dreq contents are owned by the caller. */
    goto done;

cleanup_dreq:
    /* we failed, so don't pass dreq contents to the caller. */
    dispose((disposable_t*)dreq);

done:
    return retval;
}
