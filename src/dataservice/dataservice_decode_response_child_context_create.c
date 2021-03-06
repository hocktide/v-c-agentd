/**
 * \file dataservice/dataservice_decode_response_child_context_create.c
 *
 * \brief Decode the response from the child context create api method.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

/**
 * \brief Decode a response from the child context create API call.
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
int dataservice_decode_response_child_context_create(
    const void* resp, size_t size,
    dataservice_response_child_context_create_t* dresp)
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

    /* | Child context create response packet.                             | */
    /* | -------------------------------------------------- | ------------ | */
    /* | DATA                                               | SIZE         | */
    /* | -------------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE     | 4 bytes      | */
    /* | offset                                             | 4 bytes      | */
    /* | status                                             | 4 bytes      | */
    /* | child_context_index                                | 4 bytes      | */
    /* | -------------------------------------------------- | ------------ | */

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
    if (size < response_packet_size)
    {
        retval = AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE;
        goto done;
    }

    /* verify that the method code is the code we expect. */
    dresp->hdr.method_code = ntohl(val[0]);
    if (DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE !=
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
    dresp->hdr.payload_size = 0;

    /* if the call was successful, decode the child id. */
    if (AGENTD_STATUS_SUCCESS == dresp->hdr.status)
    {
        /* verify the increased payload size. */
        if (size < response_packet_size + sizeof(uint32_t))
        {
            retval =
                AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_DATA_PACKET_SIZE;
            goto done;
        }

        /* get the child index. */
        dresp->child = ntohl(val[3]);

        /* set the payload size. */
        dresp->hdr.payload_size = sizeof(*dresp) - sizeof(dresp->hdr);
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

    /* fall-through. */

done:
    return retval;
}
