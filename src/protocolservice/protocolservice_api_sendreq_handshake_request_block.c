/**
 * \file protocolservice/protocolservice_api_sendreq_handshake_request_block.c
 *
 * \brief Write a handshake request to the peer.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/ipc.h>
#include <agentd/protocolservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Send a handshake request to the API.
 *
 * \param sock              The socket to which this request is written.
 * \param suite             The crypto suite to use for this handshake.
 * \param entity_id         The entity UUID originating this request.
 * \param key_nonce         Buffer to receive the client key nonce for this
 *                          request.  This buffer must not have been previously
 *                          initialized.  On success, this is owned by the
 *                          caller and must be disposed.
 * \param challenge_nonce   Buffer to receive the client challenge nonce for
 *                          this request.  This buffer must not have been
 *                          previously initialized.  On success, this is owned
 *                          by the caller and must be disposed.
 *
 * This function generates entropy data for the nonces based on the suite.  This
 * data is passed to the peer on the other end of the socket.  On a successful
 * return from this function, the key_nonce and challenge_nonce buffers are
 * initialized with this entropy data and must be disposed by the caller.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if thsi operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_handshake_request_block(
    int sock, vccrypt_suite_options_t* suite, const uint8_t* entity_id,
    vccrypt_buffer_t* key_nonce, vccrypt_buffer_t* challenge_nonce)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(NULL != suite);
    MODEL_ASSERT(NULL != entity_id);
    MODEL_ASSERT(NULL != key_nonce);
    MODEL_ASSERT(NULL != challenge_nonce);

    /* | Handshake request packet.                                          | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATA                                                | SIZE         | */
    /* | --------------------------------------------------- | ------------ | */
    /* | UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE           |  4 bytes     | */
    /* | offset                                              |  4 bytes     | */
    /* | record:                                             | 88 bytes     | */
    /* |    protocol_version                                 |  4 bytes     | */
    /* |    crypto_suite                                     |  4 bytes     | */
    /* |    entity_id                                        | 16 bytes     | */
    /* |    client key nonce                                 | 32 bytes     | */
    /* |    client challenge nonce                           | 32 bytes     | */
    /* | --------------------------------------------------- | ------------ | */

    /* create prng. */
    vccrypt_prng_context_t prng;
    retval = vccrypt_suite_prng_init(suite, &prng);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* initialize key nonce buffer. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            suite, key_nonce);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_prng;
    }

    /* read key nonce from prng. */
    retval = vccrypt_prng_read(&prng, key_nonce, key_nonce->size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_key_nonce;
    }

    /* initialize challenge nonce buffer. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            suite, challenge_nonce);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_key_nonce;
    }

    /* read challenge nonce from prng. */
    retval = vccrypt_prng_read(&prng, challenge_nonce, challenge_nonce->size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_challenge_nonce;
    }

    /* compute the payload size. */
    uint32_t request = htonl(UNAUTH_PROTOCOL_REQ_ID_HANDSHAKE_INITIATE);
    uint32_t offset = htonl(0);
    uint32_t protocol_version = htonl(0x01);
    uint32_t crypto_suite = htonl(VCCRYPT_SUITE_VELO_V1);
    size_t payload_size =
        sizeof(request) + sizeof(offset) + sizeof(protocol_version) + sizeof(crypto_suite) + 16 /* entity_id */
        + key_nonce->size + challenge_nonce->size;

    /* create handshake request payload buffer. */
    vccrypt_buffer_t payload;
    retval = vccrypt_buffer_init(&payload, suite->alloc_opts, payload_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_challenge_nonce;
    }

    /* write request value to payload buffer. */
    uint8_t* pbuf = (uint8_t*)payload.data;
    memcpy(pbuf, &request, sizeof(request));
    pbuf += sizeof(request);

    /* write offset value to payload buffer. */
    memcpy(pbuf, &offset, sizeof(offset));
    pbuf += sizeof(offset);

    /* write the protocol version to the payload buffer. */
    memcpy(pbuf, &protocol_version, sizeof(protocol_version));
    pbuf += sizeof(protocol_version);

    /* write the crypto suite to the payload buffer. */
    memcpy(pbuf, &crypto_suite, sizeof(crypto_suite));
    pbuf += sizeof(crypto_suite);

    /* write entity id to payload buffer. */
    memcpy(pbuf, entity_id, 16);
    pbuf += 16;

    /* write client key nonce to payload buffer. */
    memcpy(pbuf, key_nonce->data, key_nonce->size);
    pbuf += key_nonce->size;

    /* write client challenge nonce to payload buffer. */
    memcpy(pbuf, challenge_nonce->data, challenge_nonce->size);
    pbuf += challenge_nonce->size;

    /* write data packet with request payload to socket. */
    retval = ipc_write_data_block(sock, payload.data, payload.size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_payload;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_payload:
    dispose((disposable_t*)&payload);

cleanup_challenge_nonce:
    if (retval != AGENTD_STATUS_SUCCESS)
    {
        dispose((disposable_t*)challenge_nonce);
    }

cleanup_key_nonce:
    if (retval != AGENTD_STATUS_SUCCESS)
    {
        dispose((disposable_t*)key_nonce);
    }

cleanup_prng:
    dispose((disposable_t*)&prng);

done:
    return retval;
}
