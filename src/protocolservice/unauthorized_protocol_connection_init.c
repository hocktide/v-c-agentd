/**
 * \file protocolservice/unauthorized_protocol_connection_init.c
 *
 * \brief Initialize an unauthorized protocol connection.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/* forward decls. */
static void unauthorized_protocol_connection_dispose(void* disposable);

/**
 * \brief Initialize an unauthorized protocol connection instance.
 *
 * This instance takes ownership of the socket, which will be closed when this
 * instance is disposed.  This is different than the default behavior of
 * ipc_make_noblock.
 *
 * \param conn          The connection to initialize.
 * \param sock          The socket descriptor for this instance.
 * \param svc           The unauthorized protocol service instance.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE if a failure
 *        occurred when attempting to make a non-blocking socket.
 */
int unauthorized_protocol_connection_init(
    unauthorized_protocol_connection_t* conn, int sock,
    unauthorized_protocol_service_instance_t* svc)
{
    int retval = AGENTD_STATUS_SUCCESS;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != conn);
    MODEL_ASSERT(sock > 0);

    /* clear the connection. */
    memset(conn, 0, sizeof(unauthorized_protocol_connection_t));

    /* initialize the data for this instance. */
    conn->hdr.dispose = &unauthorized_protocol_connection_dispose;
    conn->state = UPCS_READ_HANDSHAKE_REQ_FROM_CLIENT;
    conn->svc = svc;

    /* by default, a connection is not associated with a dataservice
     * child context. */
    conn->dataservice_child_context = -1;

    /* attempt to make this socket non-blocking. */
    retval = ipc_make_noblock(sock, &conn->ctx, conn);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto done;
    }

    /* create buffer for the entity public key. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_public_key(
            &conn->svc->suite, &conn->entity_public_key);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create buffer for client key nonce. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &conn->svc->suite, &conn->client_key_nonce);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_public_key_buffer;
    }

    /* create buffer for client challenge nonce. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &conn->svc->suite, &conn->client_challenge_nonce);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_key_nonce_buffer;
    }

    /* create buffer for server key nonce. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &conn->svc->suite, &conn->server_key_nonce);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_challenge_nonce_buffer;
    }

    /* create buffer for server challenge nonce. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &conn->svc->suite, &conn->server_challenge_nonce);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_server_key_nonce_buffer;
    }

    /* create buffer for shared secret. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_shared_secret(
            &conn->svc->suite, &conn->shared_secret);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_server_challenge_nonce_buffer;
    }

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

cleanup_server_challenge_nonce_buffer:
    dispose((disposable_t*)&conn->server_challenge_nonce);

cleanup_server_key_nonce_buffer:
    dispose((disposable_t*)&conn->server_key_nonce);

cleanup_challenge_nonce_buffer:
    dispose((disposable_t*)&conn->client_challenge_nonce);

cleanup_key_nonce_buffer:
    dispose((disposable_t*)&conn->client_key_nonce);

cleanup_public_key_buffer:
    dispose((disposable_t*)&conn->entity_public_key);

done:
    return retval;
}

/**
 * \brief Dispose of a connection instance.
 *
 * \param disposable        The instance to dispose.
 */
static void unauthorized_protocol_connection_dispose(void* disposable)
{
    unauthorized_protocol_connection_t* conn =
        (unauthorized_protocol_connection_t*)disposable;

    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != conn);

    /* dispose of the socket context. */
    dispose((disposable_t*)&conn->ctx);

    /* dispose of the crypto buffers. */
    dispose((disposable_t*)&conn->shared_secret);
    dispose((disposable_t*)&conn->server_challenge_nonce);
    dispose((disposable_t*)&conn->server_key_nonce);
    dispose((disposable_t*)&conn->client_challenge_nonce);
    dispose((disposable_t*)&conn->client_key_nonce);
    dispose((disposable_t*)&conn->entity_public_key);

    /* clean up the instance. */
    memset(conn, 0, sizeof(unauthorized_protocol_connection_t));
}
