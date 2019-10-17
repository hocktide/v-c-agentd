/**
 * \file dataservice/dataservice_api_recvresp_child_context_create.c
 *
 * \brief Read the response from the child context create call.
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
 * \brief Receive a response from the child context create API call.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 * \param child         The integer index of the created child context,
 *                      populated if status is 0.
 *
 * On a successful return from this function, the status and child index are
 * updated with the status code from the API request.  This status should be
 * checked.  A zero status indicates success, and a non-zero status indicates
 * failure.  The child index is only good if status is 0.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.  Here are a
 * few possible status codes; it is not possible to list them all.
 *      - AGENTD_STATUS_SUCCESS if the remote operation completed successfully.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if this client node is not
 *        authorized to perform the requested operation.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet size is invalid.
 *      - AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_MAX_REACHED if no more child
 *        contexts can be created.
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
int dataservice_api_recvresp_child_context_create_block(
    int sock, uint32_t* offset, uint32_t* status, uint32_t* child)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != status);

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
    dataservice_response_child_context_create_t dresp;
    retval =
        dataservice_decode_response_child_context_create(val, size, &dresp);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_val;
    }

    /* get the offset. */
    *offset = dresp.hdr.offset;

    /* get the status code. */
    *status = dresp.hdr.status;

    /* if the call was successful, set the child id. */
    if (AGENTD_STATUS_SUCCESS == dresp.hdr.status)
    {
        /* get the child context index. */
        *child = dresp.child;
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
