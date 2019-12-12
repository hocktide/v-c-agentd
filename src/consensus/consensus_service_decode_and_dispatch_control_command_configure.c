/**
 * \file
 * consensus/consensus_service_decode_and_dispatch_control_command_configure.c
 *
 * \brief Decode and dispatch the configure command.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/consensusservice.h>
#include <agentd/consensusservice/api.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "consensusservice_internal.h"

/**
 * \brief Decode and dispatch a configure request.
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
int consensus_service_decode_and_dispatch_control_command_configure(
    consensusservice_instance_t* instance, ipc_socket_context_t* sock,
    const void* req, size_t size)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != instance);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* if this instance has already been configured, then it can't be
     * re-configured. */
    if (instance->configured)
    {
        consensus_service_decode_and_dispatch_write_status(
            sock, CONSENSUSSERVICE_API_METHOD_START, 0U,
            AGENTD_ERROR_CONSENSUSSERVICE_ALREADY_CONFIGURED, NULL, 0);

        return AGENTD_STATUS_SUCCESS;
    }

    /* calculate the payload size. */
    const size_t payload_size = 2 * sizeof(uint64_t);

    /* verify that the size is correct. */
    if (payload_size != size)
    {
        consensus_service_decode_and_dispatch_write_status(
            sock, CONSENSUSSERVICE_API_METHOD_START, 0U,
            AGENTD_ERROR_CONSENSUSSERVICE_REQUEST_PACKET_INVALID_SIZE, NULL, 0);

        return AGENTD_ERROR_CONSENSUSSERVICE_REQUEST_PACKET_INVALID_SIZE;
    }

    /* make working with the payload more convenient. */
    const uint8_t* breq = (const uint8_t*)req;

    /* get the sleep seconds. */
    uint64_t net_sleep_seconds;
    memcpy(&net_sleep_seconds, breq, sizeof(net_sleep_seconds));

    /* get the max transactions. */
    uint64_t net_max_transactions;
    memcpy(
        &net_max_transactions, breq + sizeof(net_sleep_seconds),
        sizeof(net_max_transactions));

    /* save the configuration data. */
    instance->block_max_seconds = ntohll(net_sleep_seconds);
    instance->block_max_transactions = ntohll(net_max_transactions);
    instance->configured = true;

    /* write a success status. */
    consensus_service_decode_and_dispatch_write_status(
        sock, CONSENSUSSERVICE_API_METHOD_CONFIGURE, 0U, AGENTD_STATUS_SUCCESS,
        NULL, 0);

    return AGENTD_STATUS_SUCCESS;
}
