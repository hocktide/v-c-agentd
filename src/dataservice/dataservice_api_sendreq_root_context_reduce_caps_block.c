/**
 * \file dataservice/dataservice_api_sendreq_root_context_reduce_caps_block.c
 *
 * \brief Request that the capabilities of the root context be reduced, using a
 * blocking socket.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/private/dataservice.h>
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
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
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
        return 1;
    }

    /* allocate a structure large enough for writing this request. */
    size_t reqbuflen = size + sizeof(uint32_t);
    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return 1;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_REDUCE_CAPS);
    memcpy(reqbuf, &req, sizeof(req));

    /* copy the capabilities parameter to this buffer. */
    memcpy(reqbuf + sizeof(req), caps, size);

    /* write the data packet. */
    int retval = ipc_write_data_block(sock, reqbuf, reqbuflen);

    /* clean up memory. */
    memset(reqbuf, 0, reqbuflen);
    free(reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
