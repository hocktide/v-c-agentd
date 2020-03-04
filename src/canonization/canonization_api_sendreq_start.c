/**
 * \file canonization/canonization_api_sendreq_start.c
 *
 * \brief Start the canonization service.
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Start the canonization service.
 *
 * \param sock          The socket on which this request is made.
 *
 * This call starts the canonization service, and must occur after it has been
 * successfully configured.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_WRITE_DATA_FAILURE if an error
 *        occurred when writing to the socket.
 */
int canonization_api_sendreq_start(
    int sock)
{
    /* | Canonization service start request packet.                   | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATA                                          | SIZE         | */
    /* | --------------------------------------------- | ------------ | */
    /* | CANONIZATIONSERVICE_API_METHOD_START          | 4 bytes      | */
    /* | --------------------------------------------- | ------------ | */

    /* compute the request buffer length. */
    size_t reqbuflen = sizeof(uint32_t);

    /* allocate the request buffer. */
    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(CANONIZATIONSERVICE_API_METHOD_START);
    memcpy(reqbuf, &req, sizeof(req));

    /* write the request packet to the control socket. */
    int retval = ipc_write_data_block(sock, reqbuf, reqbuflen);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_CANONIZATIONSERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up memory. */
    memset(reqbuf, 0, reqbuflen);
    free(reqbuf);

    /* return the status of the socket write to the caller. */
    return retval;
}
