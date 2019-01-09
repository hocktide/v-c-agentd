/**
 * \file dataservice/dataservice_decode_and_dispatch_write_status.c
 *
 * \brief Write the status code from a dataservice method to the caller's
 * socket.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Write a status response to the socket.
 *
 * Returns 0 on success or non-fatal error.  If a non-zero error message is
 * returned, then a fatal error has occurred that should not be recovered from.
 *
 * \param sock          The socket on which the request was received and the
 *                      response is to be written.
 * \param method        The API method of this request.
 * \param offset        The offset for the child context.
 * \param status        The status returned from this API method.
 * \param data          Additional payload data for this call.  May be NULL.
 * \param data_size     The size of this additional payload data.  Must be 0 if
 *                      data is NULL.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE if data could not be
 *        written to the client socket.
 */
int dataservice_decode_and_dispatch_write_status(
    ipc_socket_context_t* sock, uint32_t method, uint32_t offset,
    uint32_t status, void* data, size_t data_size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);

    /* | Response packet.                                             | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATA                                          | SIZE         | */
    /* | --------------------------------------------- | ------------ | */
    /* | method_id                                     | 4 bytes      | */
    /* | offset                                        | 4 bytes      | */
    /* | status                                        | 4 bytes      | */
    /* | data                                          | n - 12 bytes | */
    /* | --------------------------------------------- | ------------ | */

    /* compute the size of the response. */
    size_t respsize =
        /* the size of the method */
        sizeof(uint32_t) +
        /* the size of the offset */
        sizeof(uint32_t) +
        /* the size of the status */
        sizeof(uint32_t);

    /* add the data size to this buffer. */
    if (data != NULL)
    {
        respsize += data_size;
    }

    /* allocate memory for the response. */
    uint32_t* resp = (uint32_t*)malloc(respsize);
    if (NULL == resp)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* set the values for the response. */
    resp[0] = htonl(method);
    resp[1] = htonl(offset);
    resp[2] = htonl(status);

    /* copy the data. */
    if (data != NULL)
    {
        memcpy(resp + 3, data, data_size);
    }

    /* write the data packet. */
    int retval = ipc_write_data_noblock(sock, resp, respsize);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up memory. */
    memset(resp, 0, respsize);
    free(resp);

    /* return the status of the response write to the caller. */
    return retval;
}
