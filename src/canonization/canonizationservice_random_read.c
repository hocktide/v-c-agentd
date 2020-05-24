/**
 * \file canonization/canonizationservice_random_read.c
 *
 * \brief Read data from the random service socket from the canonization service
 * socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/randomservice.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Handle read events on the random socket.
 *
 * \param ctx               The non-blocking socket context.
 * \param event_flags       The event that triggered this callback.
 * \param user_context      The user context for this control socket.
 */
void canonizationservice_random_read(
    ipc_socket_context_t* ctx, int UNUSED(event_flags), void* user_context)
{
    uint32_t* resp = NULL;
    uint32_t resp_size = 0U;

    /* get the instance from the user context. */
    canonizationservice_instance_t* instance =
        (canonizationservice_instance_t*)user_context;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(event_flags & IPC_SOCKET_EVENT_READ);
    MODEL_ASSERT(NULL != instance);

    /* don't process data from this socket if we have been forced to exit. */
    if (instance->force_exit)
        return;

    /* attempt to read a response packet. */
    int retval = ipc_read_data_noblock(ctx, (void**)&resp, &resp_size);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK == retval)
    {
        goto done;
    }
    /* handle general failures from the data service socket read. */
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        canonizationservice_exit_event_loop(instance);
        goto done;
    }

    /* sanity check.  We should be in the block id read state. */
    if (CANONIZATIONSERVICE_STATE_WAITRESP_GET_RANDOM_BYTES != instance->state)
    {
        canonizationservice_exit_event_loop(instance);
        goto cleanup_resp;
    }

    /* verify the size of the response packet. */
    if (resp_size < 3 * sizeof(uint32_t))
    {
        canonizationservice_exit_event_loop(instance);
        goto cleanup_resp;
    }

    uint32_t method_id = ntohl(resp[0]);
    //uint32_t offset = ntohl(resp[1]);
    uint32_t status = ntohl(resp[2]);
    void* data = (void*)(resp + 3);
    size_t data_size = resp_size - 3 * sizeof(uint32_t);

    /* sanity check of response from random read. */
    if (
        method_id != RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES || status != AGENTD_STATUS_SUCCESS || data_size != 16)
    {
        canonizationservice_exit_event_loop(instance);
        goto cleanup_resp;
    }

    /* save the new block UUID. */
    memcpy(instance->block_id, data, 16);

    /* create the child context. */
    retval =
        canonizationservice_dataservice_sendreq_child_context_create(instance);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        canonizationservice_exit_event_loop(instance);
        goto cleanup_resp;
    }

    /* success. */

cleanup_resp:
    memset(resp, 0, resp_size);
    free(resp);

done:;
}
