/**
 * \file protocolservice/unauthorized_protocol_service_write_entropy_request.c
 *
 * \brief Write an entropy gathering request to the random service.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice.h>
#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Write a request to the random service to gather entropy.
 *
 * \param conn          The connection for which the request is written.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero return code on failure.
 */
int unauthorized_protocol_service_write_entropy_request(
    unauthorized_protocol_connection_t* conn)
{
    /* TODO - replace with random API method. */
    uint32_t payload[3] = {
        htonl(RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES),
        htonl(conn - conn->svc->connections),
        htonl(conn->server_key_nonce.size + conn->server_challenge_nonce.size)
    };

    /* attempt to write the request payload to the random socket. */
    int retval =
        ipc_write_data_noblock(&conn->svc->random, payload, sizeof(payload));
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        return AGENTD_ERROR_PROTOCOLSERVICE_PRNG_REQUEST_FAILURE;
    }

    /* set state to gathering entropy. */
    conn->state = UPCS_HANDSHAKE_GATHER_ENTROPY;

    /* set the write callback for the random socket. */
    ipc_set_writecb_noblock(
        &conn->svc->random, &unauthorized_protocol_service_random_write,
        &conn->svc->loop);

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
