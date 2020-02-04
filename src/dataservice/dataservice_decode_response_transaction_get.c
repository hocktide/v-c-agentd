/**
 * \file dataservice/dataservice_decode_response_transaction_get.c
 *
 * \brief Decode the response from the transaction get api method.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

/**
 * \brief Decode a response from the get transaction query.
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
int dataservice_decode_response_transaction_get(
    const void* resp, size_t size,
    dataservice_response_transaction_get_t* dresp)
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

    /* | Transaction get response packet.                                   | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATA                                                | SIZE         | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ      |  4 bytes     | */
    /* | offset                                              |  4 bytes     | */
    /* | status                                              |  4 bytes     | */
    /* | node:                                               | 64 bytes     | */
    /* |    key                                              | 16 bytes     | */
    /* |    prev                                             | 16 bytes     | */
    /* |    next                                             | 16 bytes     | */
    /* |    artifact_id                                      | 16 bytes     | */
    /* | data                                                | n - 76 bytes | */
    /* | --------------------------------------------------- | ------------ | */

    /* clear dresp. */
    memset(dresp, 0, sizeof(*dresp));

    /* by default, the disposer is the memset disposer. */
    dresp->hdr.hdr.dispose = &dataservice_decode_response_memset_disposer;
    dresp->hdr.payload_size = 0U;

    /* val is easier to work with. */
    const uint32_t* val = (const uint32_t*)resp;

    /* the size of the id array. */
    uint32_t id_arr_size = 4 * 16;

    /* set up data size for later. */
    uint32_t dat_size = size;

    /* the size should be greater than or equal to the size we expect. */
    uint32_t response_packet_size =
        /* size of the API method. */
        sizeof(uint32_t) +
        /* size of the offset. */
        sizeof(uint32_t) +
        /* size of the status. */
        sizeof(uint32_t);
    if (size < response_packet_size)
    {
        retval = AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE;
        goto done;
    }

    /* verify that the method code is the code we expect. */
    dresp->hdr.method_code = ntohl(val[0]);
    if (DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ !=
        dresp->hdr.method_code)
    {
        retval = AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE;
        goto done;
    }

    /* get the offset. */
    dresp->hdr.offset = ntohl(val[1]);

    /* get the status code. */
    dresp->hdr.status = ntohl(val[2]);
    if (AGENTD_STATUS_SUCCESS != dresp->hdr.status)
    {
        retval = AGENTD_STATUS_SUCCESS;
        goto done;
    }

    /* if successful, the size should be at least large enough to hold the
     * id array. */
    if (size < response_packet_size + id_arr_size)
    {
        retval = AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE;
        goto done;
    }

    /* get the raw data. */
    const uint8_t* bval = (const uint8_t*)(val + 3);
    /* update data size. */
    dat_size -= response_packet_size + id_arr_size;

    /* copy the key. */
    memcpy(dresp->node.key, bval, sizeof(dresp->node.key));

    /* copy the prev. */
    memcpy(dresp->node.prev, bval + 16, sizeof(dresp->node.prev));

    /* copy the next. */
    memcpy(dresp->node.next, bval + 32, sizeof(dresp->node.next));

    /* copy the artifact_id. */
    memcpy(dresp->node.artifact_id, bval + 48, sizeof(dresp->node.artifact_id));

    /* set the size. */
    dresp->node.net_txn_cert_size = htonll(dat_size);

    /* get to the location of the data. */
    bval += id_arr_size;

    /* set the data pointer. */
    dresp->data = bval;

    /* set the data size. */
    dresp->data_size = dat_size;

    /* set the payload size. */
    dresp->hdr.payload_size = sizeof(*dresp) - sizeof(dresp->hdr);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

    /* fall-through. */

done:
    return retval;
}
