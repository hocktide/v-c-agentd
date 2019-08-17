/**
 * \file dataservice/dataservice_decode_and_dispatch_global_setting_get.c
 *
 * \brief Decode and dispatch a global setting get request.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"
#include "dataservice_protocol_internal.h"

/**
 * \brief Decode and dispatch a global setting get request.
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
 *      - AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE if data could not be
 *        written to the client socket.
 */
int dataservice_decode_and_dispatch_global_setting_get(
    dataservice_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size)
{
    int retval = 0;
    char* payload_data = NULL;
    size_t payload_size = 0;

    /* TODO - come up with a more generic way to handle buffer. */
    char buffer[16384];
    size_t buffer_size = sizeof(buffer);

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* default child_index. */
    uint32_t child_index = 0U;

    /* global settings key. */
    uint64_t key = 0ULL;

    /* parse the request payload. */
    retval =
        dataservice_decode_request_global_setting_get(
            req, size, &child_index, &key);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* look up the child context. */
    dataservice_child_context_t* ctx = NULL;
    retval = dataservice_child_context_lookup(&ctx, inst, child_index);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* call the global settings get method. */
    retval = dataservice_global_settings_get(ctx, key, buffer, &buffer_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        payload_data = NULL;
        payload_size = 0;
    }
    else
    {
        /* write the data to the socket. */
        payload_data = buffer;
        payload_size = buffer_size;
    }

done:
    /* write the status to the caller. */
    retval =
        dataservice_decode_and_dispatch_write_status(
            sock, DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_READ, child_index,
            (uint32_t)retval, payload_data, payload_size);

    /* clear the buffer. */
    memset(buffer, 0, sizeof(buffer));

    return retval;
}
