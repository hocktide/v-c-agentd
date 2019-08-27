/**
 * \file dataservice/dataservice_decode_and_dispatch_block_id_by_height_read.c
 *
 * \brief Decode and dispatch the block id read by height request.
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
 * \brief Decode and dispatch a block id read by height request.
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
int dataservice_decode_and_dispatch_block_id_by_height_read(
    dataservice_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size)
{
    int retval = 0;
    bool dispose_dreq = false;
    void* payload = NULL;
    size_t payload_size = 0U;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* structure for the decoded request. */
    dataservice_request_block_id_by_height_read_t dreq;

    /* parse the request payload. */
    retval =
        dataservice_decode_request_block_id_by_height_read(req, size, &dreq);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* be sure to clean up dreq. */
    dispose_dreq = true;

    /* look up the child context. */
    dataservice_child_context_t* ctx = NULL;
    retval = dataservice_child_context_lookup(&ctx, inst, dreq.hdr.child_index);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* call the block id get by height method. */
    uint8_t block_id[16];
    retval =
        dataservice_block_id_by_height_get(
            ctx, NULL, dreq.block_height, block_id);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* encode the payload. */
    retval =
        dataservice_encode_response_block_id_by_height_read(
            &payload, &payload_size, block_id);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* success. Fall through. */

done:
    /* write the status to the caller. */
    retval =
        dataservice_decode_and_dispatch_write_status(
            sock, DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ,
            dreq.hdr.child_index, (uint32_t)retval, payload, payload_size);

    /* clean up the payload. */
    if (NULL != payload)
    {
        memset(payload, 0, payload_size);
        free(payload);
    }

    if (dispose_dreq)
    {
        dispose((disposable_t*)&dreq);
    }

    return retval;
}
