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

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be greater than or equal to the child context
     * size and the 64-bit global settings key. */
    if (size <= sizeof(uint32_t) + sizeof(uint64_t))
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

    /* get the global settings key. */
    uint64_t nkey;
    memcpy(&nkey, breq, sizeof(nkey));

    /* decode the key. */
    uint64_t key = ntohll(nkey);

    /* increment breq and decrement size. */
    breq += sizeof(uint64_t);
    size -= sizeof(uint64_t);

    /* the value size should be greater than zero. */
    MODEL_ASSERT(size > 0);

    /* call the global settings set method. */
    retval =
        dataservice_global_settings_set(
            &inst->children[child_index].ctx, key, (const char*)breq, size);

done:
    /* write the status to the caller. */
    retval =
        dataservice_decode_and_dispatch_write_status(
            sock, DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE, child_index,
            (uint32_t)retval, NULL, 0);

    return retval;
}
