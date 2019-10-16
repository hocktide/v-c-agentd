/**
 * \file dataservice/dataservice_decode_response_transaction_submit.c
 *
 * \brief Decode the response from the transaction submit api method.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

/**
 * \brief Receive a response from the transaction submit operation.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_transaction_submit(
    const void* resp, size_t size,
    dataservice_response_transaction_submit_t* dresp)
{
    int retval = 0;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != resp);
    MODEL_ASSERT(NULL != dresp);

    /* runtime sanity checks. */
    if (NULL == resp || NULL == dresp)
    {
        return AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER;
    }

    /* | Transaction submit response packet.                                | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATA                                                | SIZE         | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT    | 4 bytes      | */
    /* | offset                                              | 4 bytes      | */
    /* | status                                              | 4 bytes      | */
    /* | --------------------------------------------------- | ------------ | */

    /* by default, the disposer is the memset disposer. */
    dresp->hdr.hdr.dispose = &dataservice_decode_response_memset_disposer;
    dresp->hdr.payload_size = 0U;

    /* val is easier to work with. */
    const uint32_t* val = (const uint32_t*)resp;

    /* the size should be equal to the size we expect. */
    uint32_t response_packet_size =
        /* size of the API method. */
        sizeof(uint32_t) +
        /* size of the offset. */
        sizeof(uint32_t) +
        /* size of the status. */
        sizeof(uint32_t);
    if (size != response_packet_size)
    {
        retval = AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE;
        goto done;
    }

    /* verify that the method code is the code we expect. */
    dresp->hdr.method_code = ntohl(val[0]);
    if (DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT !=
        dresp->hdr.method_code)
    {
        retval = AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE;
        goto done;
    }

    /* get the offset. */
    dresp->hdr.offset = ntohl(val[1]);

    /* get the status code. */
    dresp->hdr.status = ntohl(val[2]);

    /* set the payload size. */
    dresp->hdr.payload_size = sizeof(*dresp) - sizeof(dresp->hdr);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

    /* fall-through. */

done:
    return retval;
}
