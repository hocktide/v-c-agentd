/**
 * \file dataservice/dataservice_decode_and_dispatch_block_read.c
 *
 * \brief Decode and dispatch the block read request.
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
 * \brief Decode and dispatch a block read request.
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
int dataservice_decode_and_dispatch_block_read(
    dataservice_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size)
{
    int retval = 0;
    uint8_t* payload_bytes = NULL;
    size_t payload_size = 0U;
    uint8_t* block_bytes = NULL;
    size_t block_size = 0U;

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

    /* copy the block_id. */
    uint8_t block_id[16];
    memcpy(block_id, breq, sizeof(block_id));

    /* increment breq and decrement size. */
    breq += sizeof(uint32_t);
    size -= sizeof(uint32_t);

    /* call the block get method. */
    data_block_node_t node;
    retval =
        dataservice_block_get(
            &inst->children[child_index].ctx, NULL, block_id, &node,
            &block_bytes, &block_size);
    if (0 != retval)
    {
        block_bytes = NULL;
        goto done;
    }

    /* create the payload. */
    payload_size = 4 * 16 + sizeof(uint64_t) + block_size;
    payload_bytes = (uint8_t*)malloc(payload_size);
    if (NULL == payload_bytes)
    {
        retval = 5;
        goto done;
    }

    /* copy the node values to the payload. */
    memcpy(payload_bytes, node.key, sizeof(node.key));
    memcpy(payload_bytes + 16, node.prev, sizeof(node.prev));
    memcpy(payload_bytes + 32, node.next, sizeof(node.next));
    memcpy(payload_bytes + 48, node.first_transaction_id,
        sizeof(node.first_transaction_id));
    memcpy(payload_bytes + 64, &node.net_block_height,
        sizeof(node.net_block_height));

    /* copy the transaction data to the payload. */
    memcpy(payload_bytes + 72, block_bytes, block_size);

    /* success. Fall through. */

done:
    /* write the status to the caller. */
    retval =
        dataservice_decode_and_dispatch_write_status(
            sock, DATASERVICE_API_METHOD_APP_BLOCK_READ,
            child_index, (uint32_t)retval, payload_bytes, payload_size);

    /* clean up payload bytes. */
    if (NULL != payload_bytes)
    {
        memset(payload_bytes, 0, payload_size);
        free(payload_bytes);
    }

    /* clean up block bytes. */
    if (NULL != block_bytes)
    {
        memset(block_bytes, 0, block_size);
        free(block_bytes);
    }

    return retval;
}
