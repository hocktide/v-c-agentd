/**
 * \file dataservice/dataservice_api_sendreq_child_context_create.c
 *
 * \brief Request the creation of a child context.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Create a child context with further reduced capabilities.
 *
 * \param sock          The socket on which this request is made.
 * \param caps          The capabilities to use for this child context.
 * \param size          The size of the capabilities in bytes.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE if an error occurred
 *        when writing to the socket.
 */
int dataservice_api_sendreq_child_context_create_block(
    int sock, uint32_t* caps, size_t size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != caps);

    /* | Child context create packet.                                 | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATA                                          | SIZE         | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE| 4 bytes      | */
    /* | caps                                          | n - 4 bytes  | */
    /* | --------------------------------------------- | ------------ | */

    /* allocate a structure large enough for writing this request. */
    size_t reqbuflen = size + sizeof(uint32_t);
    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE);
    memcpy(reqbuf, &req, sizeof(req));

    /* copy the caps parameter to this buffer. */
    memcpy(reqbuf + sizeof(req), caps, size);

    /* the request packet consists of the command and capabilities. */
    int retval = ipc_write_data_block(sock, reqbuf, reqbuflen);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up memory. */
    memset(reqbuf, 0, reqbuflen);
    free(reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
