/**
 * \file protocolservice/unauthorized_protocol_service_dataservice_read.c
 *
 * \brief Read from the data service socket.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Read data from the data service socket.
 *
 * \param ctx           The socket context for this read callback.
 * \param event_flags   The event flags that led to this callback being called.
 * \param user_context  The user context for this callback (expected: a protocol
 *                      service instance).
 */
void unauthorized_protocol_service_dataservice_read(
    ipc_socket_context_t* UNUSED(ctx), int UNUSED(event_flags),
    void* user_context)
{
    uint32_t* resp = NULL;
    uint32_t resp_size = 0U;

    /* get the instance from the user context. */
    unauthorized_protocol_service_instance_t* svc =
        (unauthorized_protocol_service_instance_t*)user_context;

    /* attempt to read a response packet. */
    int retval = ipc_read_data_noblock(&svc->data, (void**)&resp, &resp_size);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK == retval)
    {
        return;
    }
    /* handle general failures from the data service socket read. */
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        /* TODO - shut down the service. */
        return;
    }

    /* verify that the size is at least large enough for a method. */
    if (resp_size < sizeof(uint32_t))
    {
        /* TODO - shut down service on corrupt data socket? */
        goto cleanup_resp;
    }

    /* decode the method. */
    uint32_t method = ntohl(resp[0]);

    /* dispatch the method. */
    switch (method)
    {
        /* child context create response. */
        case DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE:
            ups_dispatch_dataservice_response_child_context_create(
                svc, resp, resp_size);
            break;

        /* child context close response. */
        case DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE:
            ups_dispatch_dataservice_response_child_context_close(
                svc, resp, resp_size);
            break;

        /* latest block read response. */
        case DATASERVICE_API_METHOD_APP_BLOCK_ID_LATEST_READ:
            ups_dispatch_dataservice_response_block_id_latest_read(
                svc, resp, resp_size);
            break;

        /* transaction submit response. */
        case DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT:
            ups_dispatch_dataservice_response_transaction_submit(
                svc, resp, resp_size);
            break;

        /* unknown method. */
        default:
            /* TODO - if this happens after everything is decoded, log and shut
             * down service. */
            break;
    }

cleanup_resp:
    memset(resp, 0, resp_size);
    free(resp);
}
