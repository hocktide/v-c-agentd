/**
 * \file dataservice/dataservice_decode_and_dispatch_child_context_create.c
 *
 * \brief Decode requests and dispatch a child context create call.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"
#include "dataservice_protocol_internal.h"

/**
 * \brief Decode and dispatch a child context create request.
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
int dataservice_decode_and_dispatch_child_context_create(
    dataservice_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size)
{
    int retval = 0;
    void* payload = NULL;
    size_t payload_size = 0U;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* storage for the capabilities. */
    BITCAP(caps, DATASERVICE_API_CAP_BITS_MAX);

    /* parse the request. */
    retval =
        dataservice_decode_request_child_context_create(
            req, size, caps);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* allocate a free child context. */
    int child_offset = 0;
    retval = dataservice_child_details_create(inst, &child_offset);
    if (0 != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_MAX_REACHED;
        goto done;
    }

    /* explicitly allow child context create in the child caps. */
    /* NOTE that this does not bypass root capability restrictions. */
    BITCAP_SET_TRUE(inst->children[child_offset].ctx.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* call the child context create method. */
    retval = dataservice_child_context_create(
        &inst->ctx, &inst->children[child_offset].ctx, caps);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_CHILD_CONTEXT_CREATE_FAILURE;
        goto cleanup_child_instance;
    }

    /* encode the payload. */
    retval =
        dataservice_encode_response_child_context_create(
            &payload, &payload_size, child_offset);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_child_instance;
    }

    /* success. */
    goto done;

cleanup_child_instance:
    dataservice_child_details_delete(inst, child_offset);

done:
    /* write the status to output. */
    retval =
        dataservice_decode_and_dispatch_write_status(
            sock, DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE, 0,
            (uint32_t)retval, payload, payload_size);

    /* clean up payload. */
    if (NULL != payload)
    {
        memset(payload, 0, payload_size);
        free(payload);
    }

    return retval;
}
