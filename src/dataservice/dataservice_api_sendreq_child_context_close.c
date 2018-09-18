/**
 * \file dataservice/dataservice_api_sendreq_child_context_close.c
 *
 * \brief Request the close of a child context.
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
 * \brief Close the specified child context.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index to be closed.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_child_context_close(
    ipc_socket_context_t* sock, uint32_t child)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != caps);

    /* | Child context close packet.                                  | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATA                                          | SIZE         | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE | 4 bytes      | */
    /* | child_context_index                           | 4 bytes      | */
    /* | --------------------------------------------- | ------------ | */

    /* allocate a structure large enough for writing this request. */
    size_t reqbuflen = 2 * sizeof(uint32_t);
    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return 1;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE);
    memcpy(reqbuf, &req, sizeof(req));

    /* copy the child context index parameter to the buffer. */
    uint32_t nchild = htonl(child);
    memcpy(reqbuf + sizeof(req), &nchild, sizeof(nchild));

    /* the request packet consists of the command and index. */
    int retval = ipc_write_data_noblock(sock, reqbuf, reqbuflen);

    /* clean up memory. */
    memset(reqbuf, 0, reqbuflen);
    free(reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
