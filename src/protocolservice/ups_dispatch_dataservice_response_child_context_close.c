/**
 * \file protocolservice/ups_dispatch_dataservice_response_child_context_close.c
 *
 * \brief Handle the response from the dataservice child context close request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include "unauthorized_protocol_service_private.h"

/**
 * Handle a child_context_close response.
 *
 * \param svc               The protocol service instance.
 * \param resp              The response from the child context create call.
 * \param resp_size         The size of the response.
 */
void ups_dispatch_dataservice_response_child_context_close(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size)
{
    (void)svc;
    (void)resp;
    (void)resp_size;
}
