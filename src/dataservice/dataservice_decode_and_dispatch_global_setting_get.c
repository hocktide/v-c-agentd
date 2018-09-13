/**
 * \file dataservice/dataservice_decode_and_dispatch_global_setting_get.c
 *
 * \brief Decode requests and dispatch a child context close call.
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
 * \returns 0 on success or non-fatal error.  Returns non-zero on fatal error.
 */
int dataservice_decode_and_dispatch_global_setting_get(
    dataservice_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size)
{
    int retval = 0;
    char* payload_data = NULL;
    size_t payload_size = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* default child_index. */
    uint32_t child_index = 0U;

    /* TODO - come up with a more generic way to handle buffer. */
    char buffer[16384];
    size_t buffer_size = sizeof(buffer);

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be equal to the size of a child context index and
     * the 64-bit global settings key. */
    if (size != sizeof(uint32_t) + sizeof(uint64_t))
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

    /* get the global settings key. */
    uint64_t nkey;
    memcpy(&nkey, breq, sizeof(nkey));

    /* decode the key. */
    uint64_t key = ntohll(nkey);

    /* call the child context close method. */
    retval =
        dataservice_global_settings_get(
            &inst->children[child_index].ctx, key, buffer, &buffer_size);
    if (0 != retval)
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
