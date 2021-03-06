/**
 * \file canonization/canonizationservice_write_block_id_request.c
 *
 * \brief Write a block_id request to the random service.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/randomservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Write a request to the random service to generate a block id.
 *
 * \param instance      The canonization service instance.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero return code on failure.
 */
int canonizationservice_write_block_id_request(
    canonizationservice_instance_t* instance)
{
    /* TODO - replace with random API method. */
    uint32_t payload[3] = {
        htonl(RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES),
        htonl(0),
        htonl(16)
    };

    /* attempt to write the request payload to the random socket. */
    int retval =
        ipc_write_data_noblock(instance->random, payload, sizeof(payload));
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* set the state. */
    instance->state = CANONIZATIONSERVICE_STATE_WAITRESP_GET_RANDOM_BYTES;

    /* set the write callback for the random socket. */
    ipc_set_writecb_noblock(
        instance->random, &canonizationservice_random_write,
        instance->loop_context);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
