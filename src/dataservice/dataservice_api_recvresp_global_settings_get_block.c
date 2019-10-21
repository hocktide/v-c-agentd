/**
 * \file dataservice/dataservice_api_recvresp_global_settings_get.c
 *
 * \brief Read the response from the global settings get call.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Receive a response from the global settings query.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 * \param data          Pointer to a data buffer to write the data value to.
 * \param data_size     Pointer to the size of the data buffer.  On entry, the
 *                      value this is pointed to is set to the size of the data
 *                      buffer.  On exit, if successful, this size is updated to
 *                      the size written to this buffer.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.  On
 * success, the data value and size are both updated to reflect the data read
 * from the query.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.  Here are a
 * few possible status codes; it is not possible to list them all.
 *      - AGENTD_STATUS_SUCCESS if the remote operation completed successfully.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if the requested data was not
 *        found.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this client node is not
 *        authorized to perform the requested operation.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet size is invalid.
 *      - AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_BAD_INDEX if the child context
 *        index is out of bounds.
 *      - AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_INVALID if the child context is
 *        invalid.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_IPC_READ_DATA_FAILURE if reading data from
 *        the socket failed.
 *      - AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_DATA_PACKET_SIZE if the
 *        data packet size is unexpected.
 *      - AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE if the
 *        method code was unexpected.
 *      - AGENTD_ERROR_DATASERVICE_RECVRESP_MALFORMED_PAYLOAD_DATA if the
 *        payload data was malformed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int dataservice_api_recvresp_global_settings_get_block(
    int sock, uint32_t* offset, uint32_t* status, void* data,
    size_t* data_size)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != status);
    MODEL_ASSERT(NULL != data);
    MODEL_ASSERT(NULL != data_size);

    /* read a data packet from the socket. */
    void* val = NULL;
    uint32_t size = 0U;
    retval = ipc_read_data_block(sock, &val, &size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_READ_DATA_FAILURE;
        goto done;
    }

    /* decode the response. */
    dataservice_response_global_settings_get_t dresp;
    retval =
        dataservice_decode_response_global_settings_get(val, size, &dresp);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_val;
    }

    /* verify that the data size is large enough to receive this value. */
    if (*data_size < dresp.data_size)
    {
        retval = AGENTD_ERROR_DATASERVICE_RECVRESP_MALFORMED_PAYLOAD_DATA;
        goto cleanup_dresp;
    }

    /* get the offset. */
    *offset = dresp.hdr.offset;

    /* get the status code. */
    *status = dresp.hdr.status;

    /* only copy the data if this call was successful. */
    if (AGENTD_STATUS_SUCCESS == *status)
    {
        /* set the data size. */
        *data_size = dresp.data_size;

        /* copy the data. */
        memcpy(data, dresp.data, *data_size);
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_dresp;

cleanup_dresp:
    dispose((disposable_t*)&dresp);

cleanup_val:
    memset(val, 0, size);
    free(val);

done:
    return retval;
}
