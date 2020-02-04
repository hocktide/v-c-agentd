/**
 * \file dataservice/dataservice_request_dispose.c
 *
 * \brief Dispose a dataservice request structure.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>

#include "dataservice_protocol_internal.h"

/**
 * \brief Dispose of a simple dataservice request structure.
 *
 * \param disposable        The disposable to dispose.
 */
void dataservice_request_dispose(void* disposable)
{
    dataservice_request_header_t* hdr =
        (dataservice_request_header_t*)disposable;

    MODEL_ASSERT(NULL != hdr);

    memset(hdr, 0, hdr->size);
}
