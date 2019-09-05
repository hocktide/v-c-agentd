/**
 * \file authservice/auth_service_decode_and_dispatch.c
 *
 * \brief Decode requests and dispatch them using the auth service instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "auth_service_private.h"

/**
 * \brief Decode and dispatch requests received by the auth service.
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
 *      - AGENTD_ERROR_AUTHSERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet size is invalid.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_AUTHSERVICE_IPC_WRITE_DATA_FAILURE if data could not be
 *        written to the client socket.
 */
int auth_service_decode_and_dispatch(
    auth_service_instance_t* UNUSED(inst), ipc_socket_context_t* UNUSED(sock), void* req,
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
        return AGENTD_ERROR_AUTHSERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = ntohl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    //size_t payload_size = size - sizeof(uint32_t);
    //MODEL_ASSERT(payload_size >= 0);

    /* decode the method. */
    switch (method)
    {

        /* unknown method.  Return an error. */
        default:
            /* TODO: make sure to write an error to the socket as well. */
            /*dataservice_decode_and_dispatch_write_status(
                sock, method, 0U, AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_BAD,
                NULL, 0);*/

            return AGENTD_ERROR_AUTHSERVICE_REQUEST_PACKET_BAD;
    }
}
