/**
 * \file dataservice/dataservice_api_sendreq_root_context_reduce_caps_block.c
 *
 * \brief Request that the capabilities of the root context be reduced, using a
 * blocking socket.
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
 * \brief Request that the capabilities of the root context be reduced.
 *
 * \param sock          The socket on which this request is made.
 * \param caps          The capabilities to use for the reduction.
 * \param size          The size of the capabilities in bytes.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE if the
 *        capabilities size is invalid.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if this write operation would block this
 *        thread.
 *      - AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE if an error occurred
 *        when writing to the socket.
 */
int dataservice_api_sendreq_root_context_reduce_caps_block(
    int sock, uint32_t* caps, size_t size)
{
    BITCAP(bitcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != caps);
    MODEL_ASSERT(size == sizeof(bitcaps));

    /* | Root context reduce capabilities request packet.                  | */
    /* | -------------------------------------------------- | ------------ | */
    /* | DATA                                               | SIZE         | */
    /* | -------------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_REDUCE_CAPS | 4 bytes      | */
    /* | caps                                               | n - 4 bytes  | */
    /* | -------------------------------------------------- | ------------ | */

    /* verify that caps is the correct size. */
    if (size != sizeof(bitcaps))
    {
        return AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* allocate a structure large enough for writing this request. */
    size_t reqbuflen = size + sizeof(uint32_t);
    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_REDUCE_CAPS);
    memcpy(reqbuf, &req, sizeof(req));

    /* copy the capabilities parameter to this buffer. */
    memcpy(reqbuf + sizeof(req), caps, size);

    /* write the data packet. */
    int retval = ipc_write_data_block(sock, reqbuf, reqbuflen);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK != retval && AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up memory. */
    memset(reqbuf, 0, reqbuflen);
    free(reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
