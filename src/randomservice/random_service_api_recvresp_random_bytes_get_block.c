/**
 * \file dataservice/random_service_api_recvresp_random_bytes_get_block.c
 *
 * \brief Read the response from the random bytes get call.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/randomservice.h>
#include <agentd/randomservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Receive the response from the random bytes call from the random
 * service (blocking call).
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The offset of the response.
 * \param status        The status of the response.
 * \param bytes         Pointer to receive an allocated buffer of random bytes
 *                      on success.
 * \param bytes_size    The number of bytes received in this buffer on success.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_RANDOMSERVICE_IPC_READ_DATA_FAILURE if an error occurred
 *        when reading from the socket.
 */
int random_service_api_recvresp_random_bytes_get_block(
    int sock, uint32_t* offset, uint32_t* status, void** bytes,
    size_t* bytes_size)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != status);
    MODEL_ASSERT(NULL != bytes);
    MODEL_ASSERT(NULL != bytes_size);

    /* read a data packet from the socket. */
    uint32_t* resp = NULL;
    uint32_t resp_size = 0U;
    retval = ipc_read_data_block(sock, (void*)&resp, &resp_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_RANDOMSERVICE_IPC_READ_DATA_FAILURE;
        goto done;
    }

    /* verify the size of the response packet. */
    if (resp_size < 3 * sizeof(uint32_t))
    {
        retval = AGENTD_ERROR_RANDOMSERVICE_REQUEST_PACKET_INVALID_SIZE;
        goto cleanup_resp;
    }

    /* decode response packet. */
    uint32_t method_id = ntohl(resp[0]);
    *offset = ntohl(resp[1]);
    *status = ntohl(resp[2]);
    void* data = (void*)(resp + 3);
    *bytes_size = resp_size - 3 * sizeof(uint32_t);

    /* sanity check of response from random read. */
    if (
        RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES != method_id || AGENTD_STATUS_SUCCESS != *status || 0 == *bytes_size)
    {
        retval = AGENTD_ERROR_RANDOMSERVICE_REQUEST_PACKET_BAD;
        goto cleanup_resp;
    }

    /* allocate memory for the response. */
    *bytes = malloc(*bytes_size);
    if (NULL == *bytes)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_resp;
    }

    /* copy the bytes. */
    memcpy(*bytes, data, *bytes_size);

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_resp:
    memset(resp, 0, resp_size);
    free(resp);

done:
    return retval;
}
