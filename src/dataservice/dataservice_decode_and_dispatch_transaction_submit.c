/**
 * \file dataservice/dataservice_decode_and_dispatch_transaction_submit.c
 *
 * \brief Decode transaction submit request and dispatch the call.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <agentd/ipc.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Decode and dispatch a transaction submission request.
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
 * \returns 0 on success or non-fatal error.  Returns non-zero on fatal error.
 */
int dataservice_decode_and_dispatch_transaction_submit(
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

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be greater than the child context
     * size and size of the two UUIDs. */
    if (size <= sizeof(uint32_t) + 2 * 16)
    {
        retval = 1;
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
        retval = 2;
        goto done;
    }

    /* verify that this child context is open. */
    if (NULL == inst->children[child_index].hdr.dispose)
    {
        retval = 3;
        goto done;
    }

    /* copy the transaction id. */
    uint8_t txn_id[16];
    memcpy(txn_id, breq, sizeof(txn_id));

    /* increment breq and decrement size. */
    breq += sizeof(txn_id);
    size -= sizeof(txn_id);

    /* copy the artifact id. */
    uint8_t artifact_id[16];
    memcpy(artifact_id, breq, sizeof(artifact_id));

    /* increment breq and decrement size. */
    breq += sizeof(artifact_id);
    size -= sizeof(artifact_id);

    /* the value size should be greater than zero. */
    MODEL_ASSERT(size > 0);

    /* call the transaction submit method. */
    retval =
        dataservice_transaction_submit(
            &inst->children[child_index].ctx, NULL, txn_id, artifact_id,
            breq, size);

done:
    /* write the status to the caller. */
    retval =
        dataservice_decode_and_dispatch_write_status(
            sock, DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT, child_index,
            (uint32_t)retval, NULL, 0);

    return retval;
}
