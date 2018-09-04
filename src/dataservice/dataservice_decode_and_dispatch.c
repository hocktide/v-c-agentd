/**
 * \file dataservice/dataservice_decode_and_dispatch.c
 *
 * \brief Decode requests and dispatch them using the data service instance.
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
 * \brief Decode and dispatch requests received by the data service.
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
int dataservice_decode_and_dispatch(
    dataservice_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        return 1;
    }

    /* derive the payload size. */
    size_t payload_size = size - sizeof(uint32_t);
    MODEL_ASSERT(payload_size >= 0);

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* decode the method. */
    switch (method)
    {
        /* handle root context create method. */
        case DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE:
            return dataservice_decode_and_dispatch_root_context_create(
                inst, sock, breq, payload_size);

        /* unknown method.  Return an error. */
        default:
            return 2;
    }
}
