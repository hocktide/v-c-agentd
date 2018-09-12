/**
 * \file dataservice/dataservice_api_sendreq_child_context_create.c
 *
 * \brief Request the creation of a child context.
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
 * \brief Create a child context with further reduced capabilities.
 *
 * \param sock          The socket on which this request is made.
 * \param caps          The capabilities to use for this child context.
 * \param size          The size of the capabilities in bytes.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_child_context_create(
    ipc_socket_context_t* sock, uint32_t* caps, size_t size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
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
        return 1;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE);
    memcpy(reqbuf, &req, sizeof(req));

    /* copy the caps parameter to this buffer. */
    memcpy(reqbuf + sizeof(req), caps, size);

    /* the request packet consists of the command and capabilities. */
    int retval = ipc_write_data_noblock(sock, reqbuf, reqbuflen);

    /* clean up memory. */
    memset(reqbuf, 0, reqbuflen);
    free(reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
