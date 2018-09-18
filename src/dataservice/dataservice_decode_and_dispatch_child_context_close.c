/**
 * \file dataservice/dataservice_decode_and_dispatch_child_context_close.c
 *
 * \brief Decode requests and dispatch a child context close call.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/ipc.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Decode and dispatch a child context close request.
 *
 * Returns 0 on success or non-fatal error.  If a non-zero error message is
 * returned, then a fatal error has occurred that should not be recovered from.
 * Any additional information on the socket is suspect.
 *
 * \param inst          The instance on which the dispatch occurs.
 * \param sock          The socket on which the request was received and the
 *                      response is to be written.
 * \param req           The request to be decoded and dispatched.
 * \param size          The size of the request.
 *
 * \returns 0 on success or non-fatal error.  Returns non-zero on fatal error.
 */
int dataservice_decode_and_dispatch_child_context_close(
    dataservice_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* default child_index. */
    uint32_t child_index = 0U;

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be equal to the size of a child context index. */
    if (size != sizeof(uint32_t))
    {
        retval = 1;
        goto done;
    }

    /* copy the index. */
    uint32_t nchild_index;
    memcpy(&nchild_index, breq, sizeof(uint32_t));

    /* decode the index. */
    child_index = ntohl(nchild_index);

    /* check bounds. */
    if (child_index >= DATASERVICE_MAX_CHILD_CONTEXTS)
    {
        retval = 2;
        goto done;
    }

    /* verify that this child context is open. */
    if (NULL == inst->children[child_index].hdr.dispose)
    {
        retval = 3;
        goto done;
    }

    /* call the child context close method. */
    retval = dataservice_child_context_close(&inst->children[child_index].ctx);
    if (0 != retval)
    {
        retval = 4;
        goto done;
    }

    /* clean up the child instance. */
    dataservice_child_details_delete(inst, child_index);

    /* success. */
    goto done;

done:
    /* write the status to output. */
    return dataservice_decode_and_dispatch_write_status(
        sock, DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE, child_index,
        (uint32_t)retval, NULL, 0);
}
