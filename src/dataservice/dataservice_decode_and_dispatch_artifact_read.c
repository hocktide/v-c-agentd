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
    uint8_t* payload_bytes = NULL;
    size_t payload_size = 0U;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* default child_index. */
    uint32_t child_index = 0U;

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be equal to the child context */
    if (size != (sizeof(uint32_t) + 16))
    {
        retval = AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_INVALID_SIZE;
        goto done;
    }

    /* copy the index. */
    uint32_t nchild_index;
    memcpy(&nchild_index, breq, sizeof(uint32_t));

    /* increment breq and decrement size. */
    breq += sizeof(uint32_t);
    size -= sizeof(uint32_t);

    /* decode the index. */
    child_index = ntohl(nchild_index);

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

    /* copy the artifact id. */
    uint8_t artifact_id[16];
    memcpy(artifact_id, breq, sizeof(artifact_id));

    /* increment breq and decrement size. */
    breq += sizeof(uint32_t);
    size -= sizeof(uint32_t);

    /* call the artifact get method. */
    data_artifact_record_t record;
    retval =
        dataservice_artifact_get(
            &inst->children[child_index].ctx, NULL, artifact_id, &record);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create the payload. */
    payload_size = 3 * 16 + 2 * sizeof(uint64_t) + sizeof(uint32_t);
    payload_bytes = (uint8_t*)malloc(payload_size);
    if (NULL == payload_bytes)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* copy the record values to the payload. */
    memcpy(payload_bytes, record.key, sizeof(record.key));
    memcpy(payload_bytes + 16, record.txn_first, sizeof(record.txn_first));
    memcpy(payload_bytes + 32, record.txn_latest, sizeof(record.txn_latest));
    memcpy(payload_bytes + 48, &record.net_height_first,
        sizeof(record.net_height_first));
    memcpy(payload_bytes + 56, &record.net_height_latest,
        sizeof(record.net_height_latest));
    memcpy(payload_bytes + 64, &record.net_state_latest,
        sizeof(record.net_state_latest));

    /* success. Fall through. */

done:
    /* write the status to the caller. */
    retval =
        dataservice_decode_and_dispatch_write_status(
            sock, DATASERVICE_API_METHOD_APP_ARTIFACT_READ,
            child_index, (uint32_t)retval, payload_bytes, payload_size);

    /* clean up payload bytes. */
    if (NULL != payload_bytes)
    {
        memset(payload_bytes, 0, payload_size);
        free(payload_bytes);
    }

    return retval;
}
