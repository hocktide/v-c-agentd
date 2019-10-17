/**
 * \file dataservice/dataservice_decode_response_artifact_get.c
 *
 * \brief Decode the response from the artifact get api method.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

/**
 * \brief Decode a response from the get artifact query.
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
int dataservice_decode_response_artifact_get(
    const void* resp, size_t size,
    dataservice_response_artifact_get_t* dresp)
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

    /* | Artifact get response packet.                                      | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATA                                                | SIZE         | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_APP_ARTIFACT_READ            |  4 bytes     | */
    /* | offset                                              |  4 bytes     | */
    /* | status                                              |  4 bytes     | */
    /* | record:                                             | 68 bytes     | */
    /* |    key                                              | 16 bytes     | */
    /* |    txn_first                                        | 16 bytes     | */
    /* |    txn_latest                                       | 16 bytes     | */
    /* |    net_height_first                                 |  8 bytes     | */
    /* |    net_height_latest                                |  8 bytes     | */
    /* |    net_state_latest                                 |  4 bytes     | */
    /* | --------------------------------------------------- | ------------ | */

    /* clear dresp. */
    memset(dresp, 0, sizeof(*dresp));

    /* by default, the disposer is the memset disposer. */
    dresp->hdr.hdr.dispose = &dataservice_decode_response_memset_disposer;
    dresp->hdr.payload_size = 0U;

    /* val is easier to work with. */
    const uint32_t* val = (const uint32_t*)resp;

    /* set up data size for later. */
    uint32_t dat_size = size;

    /* set up the artifact record size. */
    uint32_t artifact_record_size = 68U;

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
    if (DATASERVICE_API_METHOD_APP_ARTIFACT_READ !=
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

    /* adjust the data size. */
    dat_size -= response_packet_size;

    /* if successful, the remaining size should be the artifact record size. */
    if (artifact_record_size != dat_size)
    {
        retval = AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE;
        goto done;
    }

    /* get the raw data. */
    const uint8_t* bval = (const uint8_t*)(val + 3);

    /* copy the key. */
    memcpy(dresp->record.key, bval, sizeof(dresp->record.key));

    /* copy the first txn id. */
    memcpy(dresp->record.txn_first, bval + 16, sizeof(dresp->record.txn_first));

    /* copy the latest txn id. */
    memcpy(
        dresp->record.txn_latest, bval + 32, sizeof(dresp->record.txn_latest));

    /* copy the first height */
    memcpy(
        &dresp->record.net_height_first, bval + 48,
        sizeof(dresp->record.net_height_first));

    /* copy the latest height */
    memcpy(
        &dresp->record.net_height_latest, bval + 56,
        sizeof(dresp->record.net_height_latest));

    /* copy the latest state */
    memcpy(
        &dresp->record.net_state_latest, bval + 64,
        sizeof(dresp->record.net_state_latest));

    /* set the payload size. */
    dresp->hdr.payload_size = sizeof(*dresp) - sizeof(dresp->hdr);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

    /* fall-through. */

done:
    return retval;
}
