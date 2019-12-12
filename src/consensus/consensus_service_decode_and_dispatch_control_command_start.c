/**
 * \file consensus/consensus_service_decode_and_dispatch_control_command_start.c
 *
 * \brief Decode and dispatch the start command.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/consensusservice.h>
#include <agentd/consensusservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "consensusservice_internal.h"

/**
 * \brief Decode and dispatch a start request.
 *
 * Returns \ref AGENTD_STATUS_SUCCESS on success or non-fatal error.  If a
 * non-zero error message is returned, then a fatal error has occurred that
 * should not be recovered from. Any additional information on the socket is
 * suspect.
 *
 * \param instance      The instance on which the dispatch occurs.
 * \param sock          The socket on which the request was received and the
 *                      response is to be written.
 * \param req           The request to be decoded and dispatched.
 * \param size          The size of the request.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_IPC_WRITE_DATA_FAILURE if data could not
 *        be written to the client socket.
 */
int consensus_service_decode_and_dispatch_control_command_start(
    consensusservice_instance_t* instance, ipc_socket_context_t* sock,
    const void* UNUSED(req), size_t UNUSED(size))
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != instance);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* if this instance has not been configured, then it can't be started. */
    if (!instance->configured)
    {
        consensus_service_decode_and_dispatch_write_status(
            sock, CONSENSUSSERVICE_API_METHOD_START, 0U,
            AGENTD_ERROR_CONSENSUSSERVICE_START_BEFORE_CONFIGURE, NULL, 0);

        return AGENTD_STATUS_SUCCESS;
    }

    /* if this instance is running, then it can't be started again. */
    if (instance->running)
    {
        consensus_service_decode_and_dispatch_write_status(
            sock, CONSENSUSSERVICE_API_METHOD_START, 0U,
            AGENTD_ERROR_CONSENSUSSERVICE_ALREADY_RUNNING, NULL, 0);

        return AGENTD_STATUS_SUCCESS;
    }

    /* otherwise, start the service. */
    instance->running = true;

    /* TODO - add code here for starting the service on an event timer. */

    /* write a success status. */
    consensus_service_decode_and_dispatch_write_status(
        sock, CONSENSUSSERVICE_API_METHOD_START, 0U, AGENTD_STATUS_SUCCESS,
        NULL, 0);

    return AGENTD_STATUS_SUCCESS;
}
