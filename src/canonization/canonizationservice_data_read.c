/**
 * \file canonization/canonizationservice_data_read.c
 *
 * \brief Read data from the data service socket from the canonization service
 * socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Handle read events on the data socket.
 *
 * \param ctx               The non-blocking socket context.
 * \param event_flags       The event that triggered this callback.
 * \param user_context      The user context for this control socket.
 */
void canonizationservice_data_read(
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
        return;
    }
    /* handle general failures from the data service socket read. */
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        canonizationservice_exit_event_loop(instance);
        return;
    }

    /* verify that the size is at least large enough for a method. */
    if (resp_size < sizeof(uint32_t))
    {
        canonizationservice_exit_event_loop(instance);
        goto cleanup_resp;
    }

    /* decode the method. */
    uint32_t method = ntohl(resp[0]);

    /* dispatch the method. */
    switch (method)
    {
        /* child context create response. */
        case DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE:
            canonizationservice_dataservice_response_child_context_create(
                instance, resp, resp_size);
            break;

        /* handle child context close. */
        case DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE:
            canonizationservice_dataservice_response_child_context_close(
                instance, resp, resp_size);
            break;

        /* handle transaction pq read first. */
        case DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_FIRST_READ:
            canonizationservice_dataservice_response_transaction_first_read(
                instance, resp, resp_size);
            break;

        /* handle transaction pq read. */
        case DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ:
            canonizationservice_dataservice_response_transaction_read(
                instance, resp, resp_size);
            break;

        /* handle latest block id read. */
        case DATASERVICE_API_METHOD_APP_BLOCK_ID_LATEST_READ:
            canonizationservice_dataservice_response_latest_block_id_read(
                instance, resp, resp_size);
            break;

        /* handle block read. */
        case DATASERVICE_API_METHOD_APP_BLOCK_READ:
            canonizationservice_dataservice_response_block_read(
                instance, resp, resp_size);
            break;

        /* handle block make. */
        case DATASERVICE_API_METHOD_APP_BLOCK_WRITE:
            canonizationservice_dataservice_response_block_write(
                instance, resp, resp_size);
            break;

        /* unknown method. */
        default:
            /* TODO - if this happens, log the event. */
            canonizationservice_exit_event_loop(instance);
            break;
    }

cleanup_resp:
    memset(resp, 0, resp_size);
    free(resp);
}
