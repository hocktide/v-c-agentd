/**
 * \file protocolservice/protocolservice_api_recvresp_handshake_request_block.c
 *
 * \brief Read the response from the handshake request call.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/ipc.h>
#include <agentd/protocolservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vccrypt/compare.h>
#include <vpr/parameters.h>

/**
 * \brief Receive a handshake response from the API.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_id                 The buffer to receive the server's uuid.
 *                                  Must not have been previously initialized.
 * \param client_private_key        The client's private key.
 * \param server_public_key         The buffer to receive the server's public
 *                                  key.  Must not have been previously
 *                                  initialized.
 * \param client_key_nonce          The client key nonce for this handshake.
 * \param client_challenge_nonce    The client challenge nonce for this
 *                                  handshake.
 * \param server_challenge_nonce    The buffer to receive the server's challenge
 *                                  nonce. Must not have been previously
 *                                  initialized.
 * \param shared_secret             The buffer to receive the shared secret on
 *                                  success.  Must not have been previously
 *                                  initialized.
 * \param offset                    The offset for this response.
 * \param status                    The status for this response.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates the request to the remote peer was successful, and a
 * non-zero status indicates that the request to the remote peer failed.
 *
 * The handshake is verified, and an error is returned if this verification
 * fails.  On success, the computed shared secret is written to the
 * shared_secret parameter, which is owned by the caller and must be disposed
 * when no longer needed.  Likewise, the server id, server public key, and
 * server challenge nonce buffers are written and owned by the caller, and must
 * be disposed when no longer needed.
 *
 * If the status code is updated with an error from the service, then this error
 * will be reflected in the status variable, and a AGENTD_STATUS_SUCCESS will be
 * returned by this function.  Thus, both the return value of this function and
 * the upstream status code must be checked for correct operation.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if thsi operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_handshake_request_block(
    int sock, vccrypt_suite_options_t* suite,
    vccrypt_buffer_t* server_id,
    const vccrypt_buffer_t* client_private_key,
    vccrypt_buffer_t* server_public_key,
    const vccrypt_buffer_t* client_key_nonce,
    const vccrypt_buffer_t* client_challenge_nonce,
    vccrypt_buffer_t* server_challenge_nonce,
    vccrypt_buffer_t* shared_secret, uint32_t* offset, uint32_t* status)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(NULL != suite);
    MODEL_ASSERT(NULL != server_id);
    MODEL_ASSERT(NULL != client_private_key);
    MODEL_ASSERT(NULL != server_public_key);
    MODEL_ASSERT(NULL != client_key_nonce);
    MODEL_ASSERT(NULL != client_challenge_nonce);
    MODEL_ASSERT(NULL != shared_secret);
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != status);

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
    /* |    server public key                                |  32 bytes    | */
    /* |    server key nonce                                 |  32 bytes    | */
    /* |    server challenge nonce                           |  32 bytes    | */
    /* |    server_cr_hmac                                   |  32 bytes    | */
    /* | --------------------------------------------------- | ------------ | */

    /* read a data packet from the socket. */
    uint32_t* val = NULL;
    uint32_t size = 0U;
    retval = ipc_read_data_block(sock, (void**)&val, &size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* a byte-wise pointer is convenient. */
    uint8_t* vbuf = (uint8_t*)val;

    /* Verify that the size is at least large enough to get the status and
     * offset. */
    if (size < 3 * sizeof(uint32_t))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_RESPONSE;
        goto cleanup_val;
    }

    /* assign status and offset. */
    *status = ntohl(val[1]);
    *offset = ntohl(val[2]);

    /* if the status indicates failure, then stop. */
    if (AGENTD_STATUS_SUCCESS != *status)
    {
        retval = *status;
        goto cleanup_val;
    }

    /* create the mac options. */
    /* TODO - replace with suite short mac. */
    vccrypt_mac_options_t mac_options;
    retval =
        vccrypt_mac_options_init(
            &mac_options, suite->alloc_opts,
            VCCRYPT_MAC_ALGORITHM_SHA_2_512_256_HMAC);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_val;
    }

    /* compute the expected size of the payload. */
    size_t payload_size =
        4 /* request_id */
        + 4 /* offset */
        + 4 /* status */
        + 4 /* protocol_version */
        + 4 /* crypto_suite */
        + 16 /* agent id */
        + suite->key_cipher_opts.public_key_size + suite->key_cipher_opts.minimum_nonce_size + suite->key_cipher_opts.minimum_nonce_size + mac_options.mac_size;

    /* verify the payload size. */
    if (size != payload_size)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_RESPONSE;
        goto cleanup_mac_options;
    }

    /* verify the protocol version. */
    uint32_t protocol_version = ntohl(val[3]);
    if (0x00000001 != protocol_version)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_RESPONSE;
        goto cleanup_mac_options;
    }

    /* verify the crypto suite. */
    uint32_t crypto_suite = ntohl(val[4]);
    if (VCCRYPT_SUITE_VELO_V1 != crypto_suite)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_RESPONSE;
        goto cleanup_mac_options;
    }

    /* create buffer for agent_id. */
    retval = vccrypt_buffer_init(server_id, suite->alloc_opts, 16);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac_options;
    }

    /* copy the server id. */
    memcpy(server_id->data, vbuf + 20, server_id->size);

    /* create buffer for agent public key. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_public_key(
            suite, server_public_key);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_server_id;
    }

    /* copy the server public key from the message. */
    /* TODO - replace this with a proper attestation process for the server
     * certificate. */
    memcpy(server_public_key->data, vbuf + 36, server_public_key->size);

    /* create buffer for server key nonce. */
    vccrypt_buffer_t server_key_nonce;
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            suite, &server_key_nonce);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_server_public_key;
    }

    /* copy the server key nonce from the message. */
    memcpy(server_key_nonce.data, vbuf + 68, server_key_nonce.size);

    /* create buffer for server challenge nonce. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            suite, server_challenge_nonce);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_server_key_nonce;
    }

    /* copy the server challenge nonce from the message. */
    memcpy(
        server_challenge_nonce->data, vbuf + 100, server_challenge_nonce->size);

    /* create buffer for shared secret. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_shared_secret(
            suite, shared_secret);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_server_challenge_nonce;
    }

    /* create the key agreement instance. */
    vccrypt_key_agreement_context_t agreement;
    retval =
        vccrypt_suite_cipher_key_agreement_init(
            suite, &agreement);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_shared_secret;
    }

    /* compute shared secret. */
    retval =
        vccrypt_key_agreement_short_term_secret_create(
            &agreement, client_private_key, server_public_key,
            &server_key_nonce, client_key_nonce,
            shared_secret);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agreement;
    }

    /* create mac. */
    vccrypt_mac_context_t mac;
    retval =
        vccrypt_mac_init(&mac_options, &mac, shared_secret);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agreement;
    }

    /* digest the payload, minus the mac. */
    retval =
        vccrypt_mac_digest(
            &mac, (uint8_t*)val, payload_size - mac_options.mac_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* add the client challenge to the digest. */
    retval =
        vccrypt_mac_digest(
            &mac, (uint8_t*)client_challenge_nonce->data,
            client_challenge_nonce->size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* create buffer for holding mac output. */
    vccrypt_buffer_t mac_buffer;
    retval =
        vccrypt_buffer_init(
            &mac_buffer, suite->alloc_opts, mac_options.mac_size);
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

    /* verify that the hmac matches. */
    vbuf += payload_size - mac_options.mac_size;
    if (0 != crypto_memcmp(mac_buffer.data, vbuf, mac_options.mac_size))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_RESPONSE;
        goto cleanup_mac_buffer;
    }

    /* temporary success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_mac_buffer:
    dispose((disposable_t*)&mac_buffer);

cleanup_mac:
    dispose((disposable_t*)&mac);

cleanup_agreement:
    dispose((disposable_t*)&agreement);

cleanup_shared_secret:
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        dispose((disposable_t*)shared_secret);
    }

cleanup_server_challenge_nonce:
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        dispose((disposable_t*)server_challenge_nonce);
    }

cleanup_server_key_nonce:
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        dispose((disposable_t*)&server_key_nonce);
    }

cleanup_server_public_key:
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        dispose((disposable_t*)server_public_key);
    }

cleanup_server_id:
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        dispose((disposable_t*)server_id);
    }

cleanup_mac_options:
    dispose((disposable_t*)&mac_options);

cleanup_val:
    memset(val, 0, size);
    free(val);

done:
    return retval;
}
