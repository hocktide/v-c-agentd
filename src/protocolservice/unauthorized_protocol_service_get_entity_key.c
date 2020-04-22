/**
 * \file protocolservice/unauthorized_protocol_service_get_entity_key.c
 *
 * \brief Get the key associated with the given connection's entity id.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <stddef.h>
#include <vccrypt/compare.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Get the entity key associated with the data read during a handshake
 * request.
 *
 * \param conn          The connection for which the entity key should be
 *                      resolved.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero return code on failure.
 */
int unauthorized_protocol_service_get_entity_key(
    unauthorized_protocol_connection_t* conn)
{
    /* verify that the entity id is authorized. */
    /* TODO - this should perform a database lookup. */
    if (0 != crypto_memcmp(conn->entity_uuid, conn->svc->authorized_entity_id, 16))
    {
        return 1;
    }

    /* the entity id is valid, so copy the entity public key. */
    memcpy(
        conn->entity_public_key.data, conn->svc->authorized_entity_pubkey.data,
        conn->entity_public_key.size);

    /* success */
    return AGENTD_STATUS_SUCCESS;
}
