/**
 * \file consensus/consensus_api_recvresp_start.c
 *
 * \brief Receive a response from the consensus service start call.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/consensusservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Receive a response from the consensus service start call.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.  Here are a
 * few possible status codes; it is not possible to list them all.
 *      - AGENTD_STATUS_SUCCESS if the remote operation completed successfully.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_REQUEST_PACKET_INVALID_SIZE if the
 *        request packet size is invalid.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_IPC_READ_DATA_FAILURE if reading data
 *        from the socket failed.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_RECVRESP_UNEXPECTED_DATA_PACKET_SIZE if
 *        the data packet size is unexpected.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_RECVRESP_UNEXPECTED_METHOD_CODE if the
 *        method code was unexpected.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_RECVRESP_MALFORMED_PAYLOAD_DATA if the
 *        payload data was malformed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int consensus_api_recvresp_start(
    int sock, uint32_t* offset, uint32_t* status)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != status);

    /* | start method response packet.                                | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATA                                          | SIZE         | */
    /* | --------------------------------------------- | ------------ | */
    /* | CONSENSUSSERVICE_API_METHOD_START             | 4 bytes      | */
    /* | offset                                        | 4 bytes      | */
    /* | status                                        | 4 bytes      | */
    /* | --------------------------------------------- | ------------ | */

    /* read a data packet from the socket. */
    void* val = NULL;
    uint32_t size = 0U;
    retval = ipc_read_data_block(sock, &val, &size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_CONSENSUSSERVICE_IPC_READ_DATA_FAILURE;
        goto done;
    }

    /* work with this as a uint32_t pointer. */
    const uint32_t* uval = (const uint32_t*)val;

    /* compute the expected size. */
    uint32_t response_packet_size =
        /* size of the API method. */
        sizeof(uint32_t) +
        /* size of the offset. */
        sizeof(uint32_t) +
        /* size of the status. */
        sizeof(uint32_t);
    if (size != response_packet_size)
    {
        retval = AGENTD_ERROR_CONSENSUSSERVICE_RESPONSE_PACKET_INVALID_SIZE;
        goto cleanup_val;
    }

    /* verify the API method. */
    if (CONSENSUSSERVICE_API_METHOD_START != ntohl(uval[0]))
    {
        retval = AGENTD_ERROR_CONSENSUSSERVICE_RECVRESP_UNEXPECTED_METHOD_CODE;
        goto cleanup_val;
    }

    /* get the offset. */
    *offset = ntohl(uval[1]);

    /* get the status code. */
    *status = ntohl(uval[2]);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

    /* fall-through. */

cleanup_val:
    memset(val, 0, response_packet_size);
    free(val);

done:
    return retval;
}
