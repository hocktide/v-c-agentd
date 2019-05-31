/**
 * \file randomservice/randomservice_decode_and_dispatch.c
 *
 * \brief Decode requests and dispatch them using the random service instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/private/randomservice.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "randomservice_internal.h"

/**
 * \brief Decode and dispatch requests received by the random service.
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
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero status indicating failure.
 */
int randomservice_decode_and_dispatch(
    randomservice_root_context_t* inst, ipc_socket_context_t* sock, void* req,
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
        return AGENTD_ERROR_RANDOMSERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = ntohl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = size - sizeof(uint32_t);
    MODEL_ASSERT(payload_size >= 0);

    /* decode the method. */
    switch (method)
    {
        /* handle get random bytes call. */
        case RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES:
            return randomservice_decode_and_dispatch_get_random_bytes(
                inst, sock, breq, payload_size);

        /* unknown method.  Return an error. */
        default:
            /* make sure to write an error to the socket as well. */
            randomservice_decode_and_dispatch_write_status(
                sock, method, 0U, AGENTD_ERROR_RANDOMSERVICE_REQUEST_PACKET_BAD,
                NULL, 0);

            return AGENTD_ERROR_RANDOMSERVICE_REQUEST_PACKET_BAD;
    }
}
