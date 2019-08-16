/**
 * \file dataservice/dataservice_decode_and_dispatch_global_setting_set.c
 *
 * \brief Decode requests and dispatch a global setting set call.
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
 * \brief Decode and dispatch a global setting set request.
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
int dataservice_decode_and_dispatch_global_setting_set(
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

    /* the key. */
    uint64_t key = 0ULL;

    /* the value. */
    void* val = NULL;
    size_t val_size = 0UL;

    /* parse the request packet. */
    retval =
        dataservice_decode_request_global_setting_set(
            req, size, &child_index, &key, &val, &val_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* check bounds. */
    if (child_index >= DATASERVICE_MAX_CHILD_CONTEXTS)
    {
        retval = AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_BAD_INDEX;
        goto done;
    }

    /* verify that this child context is open. */
    if (NULL == inst->children[child_index].hdr.dispose)
    {
        retval = AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_INVALID;
        goto done;
    }

    /* the value size should be greater than zero. */
    MODEL_ASSERT(val_size > 0);

    /* call the global settings set method. */
    retval =
        dataservice_global_settings_set(
            &inst->children[child_index].ctx, key, (const char*)val, val_size);

done:
    /* write the status to the caller. */
    retval =
        dataservice_decode_and_dispatch_write_status(
            sock, DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE, child_index,
            (uint32_t)retval, NULL, 0);

    return retval;
}
