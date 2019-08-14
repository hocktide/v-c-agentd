/**
 * \file dataservice/dataservice_decode_and_dispatch_artifact_read.c
 *
 * \brief Decode and dispatch the artifact read request.
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
 * \brief Decode and dispatch an artifact read request.
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
int dataservice_decode_and_dispatch_artifact_read(
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

    /* default child_index. */
    uint32_t child_index = 0U;

    /* artifact id buffer. */
    uint8_t artifact_id[16];

    /* parse the payload. */
    retval =
        dataservice_decode_request_payload_artifact_read(
            req, size, &child_index, artifact_id);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* check child_index bounds. */
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

    /* call the artifact get method. */
    data_artifact_record_t record;
    retval =
        dataservice_artifact_get(
            &inst->children[child_index].ctx, NULL, artifact_id, &record);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* encode the payload. */
    retval =
        dataservice_encode_response_payload_artifact_read(
            &payload, &payload_size, record.key, record.txn_first,
            record.txn_latest, ntohll(record.net_height_first),
            ntohll(record.net_height_latest), ntohl(record.net_state_latest));
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* success. Fall through. */

done:
    /* write the status to the caller. */
    retval =
        dataservice_decode_and_dispatch_write_status(
            sock, DATASERVICE_API_METHOD_APP_ARTIFACT_READ,
            child_index, (uint32_t)retval, payload, payload_size);

    /* clean up payload bytes. */
    if (NULL != payload)
    {
        memset(payload, 0, payload_size);
        free(payload);
    }

    return retval;
}
