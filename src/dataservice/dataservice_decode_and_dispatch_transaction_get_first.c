/**
 * \file dataservice/dataservice_decode_and_dispatch_transaction_get_first.c
 *
 * \brief Decode transaction get first request and dispatch the call.
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
 * \brief Decode and dispatch a transaction get first data request.
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
int dataservice_decode_and_dispatch_transaction_get_first(
    dataservice_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size)
{
    int retval = 0;
    void* payload = NULL;
    size_t payload_size = 0U;
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0U;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* default child_index. */
    uint32_t child_index = 0U;

    /* decode the request payload. */
    retval =
        dataservice_decode_request_transaction_get_first(
            req, size, &child_index);
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

    /* call the transaction get first method. */
    data_transaction_node_t node;
    retval =
        dataservice_transaction_get_first(
            &inst->children[child_index].ctx, NULL, &node,
            &txn_bytes, &txn_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        txn_bytes = NULL;
        goto done;
    }

    /* create the payload. */
    retval =
        dataservice_encode_response_transaction_get_first(
            &payload, &payload_size, node.key, node.prev, node.next,
            node.artifact_id, txn_bytes, txn_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* success. Fall through. */

done:
    /* write the status to the caller. */
    retval =
        dataservice_decode_and_dispatch_write_status(
            sock, DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_FIRST_READ,
            child_index, (uint32_t)retval, payload, payload_size);

    /* clean up payload bytes. */
    if (NULL != payload)
    {
        memset(payload, 0, payload_size);
        free(payload);
    }

    /* clean up transaction bytes. */
    if (NULL != txn_bytes)
    {
        memset(txn_bytes, 0, txn_size);
        free(txn_bytes);
    }

    return retval;
}
