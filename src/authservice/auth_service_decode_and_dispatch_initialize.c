/**
 * \file authservice/auth_service_decode_and_dispatch_initialize.c
 *
 * \brief Decode requests and dispatch an initialization call.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/authservice/private/authservice.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "auth_service_private.h"

/**
 * \brief Decode and dispatch an initialization request.
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
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_AUTHSERVICE_IPC_WRITE_DATA_FAILURE if data could not be
 *        written to the client socket.
 */
int auth_service_decode_and_dispatch_initialize(
    auth_service_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size)
{
    int retval = AGENTD_STATUS_SUCCESS;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* validate the payload size */
    if (size != sizeof(inst->agent_id) + inst->agent_pubkey.size + inst->agent_privkey.size)
    {
        retval = AGENTD_ERROR_AUTHSERVICE_REQUEST_PACKET_INVALID_SIZE;
        goto done;
    }

    /* set the agent ID */
    memcpy(inst->agent_id, breq, 16);
    breq += 16;

    /* set the public key */
    memcpy(inst->agent_pubkey.data, breq, inst->agent_pubkey.size);
    breq += inst->agent_pubkey.size;

    /* set the private key */
    memcpy(inst->agent_privkey.data, breq, inst->agent_privkey.size);

done:
    /* write the status to output. */
    return auth_service_decode_and_dispatch_write_status(
        sock, AUTHSERVICE_API_METHOD_INITIALIZE, 0,
        (uint32_t)retval, NULL, 0);
}
