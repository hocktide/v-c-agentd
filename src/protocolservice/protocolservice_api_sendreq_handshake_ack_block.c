/**
 * \file protocolservice/protocolservice_api_sendreq_handshake_ack_block.c
 *
 * \brief Send the handshake acknowledge message to the server.
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
 * \brief Send a handshake acknowledge to the API.
 *
 * \param sock                      The socket to which this request is written.
 * \param suite                     The crypto suite to use for this handshake.
 * \param client_iv                 Pointer to receive updated client IV.
 * \param shared_secret             The shared secret key for this request.
 * \param server_challenge_nonce    The server challenge nonce for this request.
 *
 * This function sends the handshake acknowledgement as an authorized packet to
 * the server.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if a blocking write on the socket
 *        failed.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if thsi operation encountered an
 *        out-of-memory error.
 *      - a non-zero error response if something else has failed.
 */
int protocolservice_api_sendreq_handshake_ack_block(
    int sock, vccrypt_suite_options_t* suite, uint64_t* client_iv,
    const vccrypt_buffer_t* shared_secret,
    const vccrypt_buffer_t* server_challenge_nonce)
{
    int retval;

    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != suite);
    MODEL_ASSERT(NULL != client_iv);
    MODEL_ASSERT(NULL != shared_secret);
    MODEL_ASSERT(NULL != server_challenge_nonce);

    /* create a buffer for holding the digest. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t digest;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(
            &digest, suite->alloc_opts, suite->mac_short_opts.mac_size))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* create a mac instance for building the response to the challenge. */
    /* TODO - shared secret parameter should be const in suite. */
    vccrypt_mac_context_t mac;
    retval =
        vccrypt_suite_mac_short_init(
            suite, &mac, (vccrypt_buffer_t*)shared_secret);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_digest;
    }

    /* digest server challenge nonce. */
    retval =
        vccrypt_mac_digest(
            &mac, server_challenge_nonce->data, server_challenge_nonce->size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* finalize the digest. */
    retval = vccrypt_mac_finalize(&mac, &digest);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* set client IV. */
    *client_iv = 0x0000000000000001;

    /* write IPC authed data packet to server. */
    /* TODO - shared_secret parameter in ipc should be const. */
    retval =
        ipc_write_authed_data_block(
            sock, *client_iv, digest.data, digest.size, suite,
            (vccrypt_buffer_t*)shared_secret);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* increment client IV. */
    *client_iv += 1;

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_mac:
    dispose((disposable_t*)&mac);

cleanup_digest:
    dispose((disposable_t*)&digest);

done:
    return retval;
}
