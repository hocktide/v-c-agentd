/**
 * \file dataservice/dataservice_decode_and_dispatch.c
 *
 * \brief Decode requests and dispatch them using the data service instance.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/ipc.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Decode and dispatch requests received by the data service.
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
int dataservice_decode_and_dispatch(
    dataservice_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* the payload size should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        return 1;
    }

    /* get the method. */
    uint32_t nmethod = 0U;
    memcpy(&nmethod, breq, sizeof(uint32_t));
    uint32_t method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    size_t payload_size = size - sizeof(uint32_t);
    MODEL_ASSERT(payload_size >= 0);

    /* decode the method. */
    switch (method)
    {
        /* handle root context create method. */
        case DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE:
            return dataservice_decode_and_dispatch_root_context_create(
                inst, sock, breq, payload_size);

        /* handle root context reduce capabilites. */
        case DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_REDUCE_CAPS:
            return dataservice_decode_and_dispatch_root_context_reduce_caps(
                inst, sock, breq, payload_size);

        /* handle child context create call. */
        case DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE:
            return dataservice_decode_and_dispatch_child_context_create(
                inst, sock, breq, payload_size);

        /* handle child context close call. */
        case DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE:
            return dataservice_decode_and_dispatch_child_context_close(
                inst, sock, breq, payload_size);

        /* handle global settings get call. */
        case DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_READ:
            return dataservice_decode_and_dispatch_global_setting_get(
                inst, sock, breq, payload_size);

        /* handle global settings set call. */
        case DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE:
            return dataservice_decode_and_dispatch_global_setting_set(
                inst, sock, breq, payload_size);

        /* handle transaction submit. */
        case DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT:
            return dataservice_decode_and_dispatch_transaction_submit(
                inst, sock, breq, payload_size);

        /* handle transaction get first. */
        case DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_FIRST_READ:
            return dataservice_decode_and_dispatch_transaction_get_first(
                inst, sock, breq, payload_size);

        /* handle transaction get. */
        case DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ:
            return dataservice_decode_and_dispatch_transaction_get(
                inst, sock, breq, payload_size);

        /* handle transaction drop. */
        case DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_DROP:
            return dataservice_decode_and_dispatch_transaction_drop(
                inst, sock, breq, payload_size);

        /* handle block make. */
        case DATASERVICE_API_METHOD_APP_BLOCK_WRITE:
            return dataservice_decode_and_dispatch_block_make(
                inst, sock, breq, payload_size);

        /* unknown method.  Return an error. */
        default:
            /* make sure to write an error to the socket as well. */
            dataservice_decode_and_dispatch_write_status(
                sock, method, 0U, 0xFFFFFFFF, NULL, 0);

            return 2;
    }
}
