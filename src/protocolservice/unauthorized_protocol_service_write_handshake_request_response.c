/**
 * \file protocolservice/unauthorized_protocol_service_write_handshake_request_response.c
 *
 * \brief Write the response for the client handshake request.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Compute and write the handshake response for the handshake request.
 *
 * \param conn          The connection for which the response should be written.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero return code on failure.
 */
int unauthorized_protocol_service_write_handshake_request_response(
    unauthorized_protocol_connection_t* conn)
{
    int retval = AGENTD_STATUS_SUCCESS;

    /* | Handshake request response packet.                                 | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATA                                                | SIZE         | */
    /* | --------------------------------------------------- | ------------ | */
    /* | UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE           |   4 bytes    | */
    /* | offset                                              |   4 bytes    | */
    /* | status                                              |   4 bytes    | */
    /* | record:                                             | 152 bytes    | */
    /* |    protocol_version                                 |   4 bytes    | */
    /* |    crypto_suite                                     |   4 bytes    | */
    /* |    agent_id                                         |  16 bytes    | */
    /* |    agent public key                                 |  32 bytes    | */
    /* |    server key nonce                                 |  32 bytes    | */
    /* |    server challenge nonce                           |  32 bytes    | */
    /* |    server_cr_hmac                                   |  32 bytes    | */
    /* | --------------------------------------------------- | ------------ | */

    /* create key agreement instance. */
    vccrypt_key_agreement_context_t agreement;
    retval =
        vccrypt_suite_cipher_key_agreement_init(
            &conn->svc->suite, &agreement);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* Derive the shared secret using the key nonces. */
    retval =
        vccrypt_key_agreement_short_term_secret_create(
            &agreement, &conn->svc->agent_privkey, &conn->entity_public_key,
            &conn->server_key_nonce, &conn->client_key_nonce,
            &conn->shared_secret);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agreement;
    }

    /* Compute the response packet payload size. */
    uint32_t request_id = htonl(UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE);
    uint32_t offset = htonl(0x00U);
    uint32_t protocol_version = htonl(0x00000001);
    uint32_t crypto_suite = htonl(VCCRYPT_SUITE_VELO_V1);
    int32_t status = htonl(AGENTD_STATUS_SUCCESS);
    size_t payload_size =
        sizeof(request_id) + sizeof(offset) + sizeof(status) + sizeof(protocol_version) + sizeof(crypto_suite) + 16 /* agent id */
        + conn->svc->agent_pubkey.size + conn->server_key_nonce.size + conn->server_challenge_nonce.size + conn->svc->suite.mac_short_opts.mac_size;

    /* Create the response packet payload buffer. */
    vccrypt_buffer_t payload;
    retval =
        vccrypt_buffer_init(&payload, &conn->svc->alloc_opts, payload_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agreement;
    }

    /* convenience pointer for working with this buffer. */
    uint8_t* buf_start = (uint8_t*)payload.data;
    uint8_t* pbuf = (uint8_t*)payload.data;

    /* write the payload values to this buffer. */
    memcpy(pbuf, &request_id, sizeof(request_id));
    pbuf += sizeof(request_id);
    memcpy(pbuf, &offset, sizeof(offset));
    pbuf += sizeof(offset);
    memcpy(pbuf, &status, sizeof(status));
    pbuf += sizeof(status);
    memcpy(pbuf, &protocol_version, sizeof(protocol_version));
    pbuf += sizeof(protocol_version);
    memcpy(pbuf, &crypto_suite, sizeof(crypto_suite));
    pbuf += sizeof(crypto_suite);
    memcpy(pbuf, conn->svc->agent_id, 16);
    pbuf += 16;
    memcpy(pbuf, conn->svc->agent_pubkey.data, conn->svc->agent_pubkey.size);
    pbuf += conn->svc->agent_pubkey.size;
    memcpy(pbuf, conn->server_key_nonce.data, conn->server_key_nonce.size);
    pbuf += conn->server_key_nonce.size;
    memcpy(
        pbuf, conn->server_challenge_nonce.data,
        conn->server_challenge_nonce.size);
    pbuf += conn->server_challenge_nonce.size;

    /* create an HMAC instance. */
    vccrypt_mac_context_t mac;
    retval =
        vccrypt_suite_mac_short_init(
            &conn->svc->suite, &mac, &conn->shared_secret);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_payload;
    }

    /* digest the response packet.*/
    retval = vccrypt_mac_digest(&mac, buf_start, pbuf - buf_start);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* add the client challenge nonce to the response packet. */
    retval =
        vccrypt_mac_digest(
            &mac, conn->client_challenge_nonce.data,
            conn->client_challenge_nonce.size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* create buffer for holding mac output. */
    /* TODO - there should be a suite buffer init method for this. */
    vccrypt_buffer_t mac_buffer;
    retval =
        vccrypt_buffer_init(
            &mac_buffer, &conn->svc->alloc_opts,
            conn->svc->suite.mac_short_opts.mac_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* finalize the mac. */
    retval = vccrypt_mac_finalize(&mac, &mac_buffer);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac_buffer;
    }

    /* copy the hmac to the payload. */
    memcpy(pbuf, mac_buffer.data, mac_buffer.size);
    pbuf += mac_buffer.size;

    /* write packet to connection and set write callback. */
    retval = ipc_write_data_noblock(&conn->ctx, payload.data, payload.size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac_buffer;
    }

    /* set state to UPCS_WRITE_HANDSHAKE_RESP_TO_CLIENT */
    conn->state = UPCS_WRITE_HANDSHAKE_RESP_TO_CLIENT;

    /* set the write callback. */
    ipc_set_writecb_noblock(
        &conn->ctx, &unauthorized_protocol_service_connection_write,
        &conn->svc->loop);

    /* set the IVs. */
    conn->client_iv = 0x0000000000000001UL;
    conn->server_iv = 0x8000000000000001UL;

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_mac_buffer:
    dispose((disposable_t*)&mac_buffer);

cleanup_mac:
    dispose((disposable_t*)&mac);

cleanup_payload:
    dispose((disposable_t*)&payload);

cleanup_agreement:
    dispose((disposable_t*)&agreement);

done:
    return retval;
}
