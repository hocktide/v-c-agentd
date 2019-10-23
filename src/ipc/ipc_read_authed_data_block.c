/**
 * \file ipc/ipc_read_authed_data_block.c
 *
 * \brief Blocking read of an authenticated data packet value.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>
#include <vccrypt/compare.h>
#include <vpr/parameters.h>

/**
 * \brief Read an authenticated data packet from the blocking socket.
 *
 * On success, an authenticated data buffer is allocated and read, along with
 * type information and size.  The caller owns this buffer and is responsible
 * for freeing it when it is no longer in use.
 *
 * \param sd            The socket descriptor from which the value is read.
 * \param iv            The 64-bit IV to expect for this packet.
 * \param val           Pointer to the pointer of the raw data buffer.
 * \param size          Pointer to the variable to receive the size of this
 *                      packet.
 * \param suite         The crypto suite to use for authenticating this packet.
 * \param secret        The shared secret between the peer and host.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_CRYPTO_SUITE if the crypto suite is
 *        invalid.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_SECRET if the secret key is invalid.
 *      - AGENTD_ERROR_IPC_AUTHENTICATION_FAILURE if the packet could not be
 *        authenticated.
 */
int ipc_read_authed_data_block(
    int sock, uint64_t iv, void** val, uint32_t* size,
    vccrypt_suite_options_t* suite, vccrypt_buffer_t* secret)
{
    int retval = 0;
    uint8_t type = 0U;
    uint32_t nsize = 0U;
    uint8_t* header = NULL;
    uint8_t* dheader = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(sock >= 0);
    MODEL_ASSERT(NULL != val);
    MODEL_ASSERT(NULL != size);
    MODEL_ASSERT(NULL != suite);
    MODEL_ASSERT(NULL != secret);

    /* attempt to allocate space for the header. */
    size_t header_size =
        sizeof(type) + sizeof(nsize) + suite->mac_short_opts.mac_size;
    vccrypt_buffer_t hbuffer;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(&hbuffer, suite->alloc_opts, header_size))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* attempt to allocate space for the decrypted header. */
    size_t dheader_size =
        sizeof(type) + sizeof(nsize);
    vccrypt_buffer_t dhbuffer;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(&dhbuffer, suite->alloc_opts, dheader_size))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_hbuffer;
    }

    /* set up pointers for convenience. */
    header = (uint8_t*)hbuffer.data;
    dheader = (uint8_t*)dhbuffer.data;

    /* attempt to read the header. */
    if ((ssize_t)header_size != read(sock, header, header_size))
    {
        retval = AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;
        goto cleanup_dhbuffer;
    }

    /* set up the stream cipher. */
    vccrypt_stream_context_t stream;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_suite_stream_init(suite, &stream, secret))
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_dhbuffer;
    }

    /* set up the MAC. */
    vccrypt_mac_context_t mac;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_suite_mac_short_init(suite, &mac, secret))
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_stream;
    }

    /* start decryption of the stream. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_stream_continue_decryption(&stream, &iv, sizeof(iv), 0))
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_mac;
    }

    /* decrypt enough of the header to determine the type and size. */
    size_t offset = 0;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_stream_decrypt(
            &stream, hbuffer.data, dheader_size, dhbuffer.data, &offset))
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_mac;
    }

    /* verify that the type is IPC_DATA_TYPE_AUTHED_PACKET. */
    type = dheader[0];
    if (IPC_DATA_TYPE_AUTHED_PACKET != type)
    {
        retval = AGENTD_ERROR_IPC_UNAUTHORIZED_PACKET;
        goto cleanup_mac;
    }

    /* verify that the size makes sense. */
    memcpy(&nsize, dheader + 1, sizeof(nsize));
    *size = ntohl(nsize);
    if (*size > 10ULL * 1024ULL * 1024ULL /* 10 MB */)
    {
        retval = AGENTD_ERROR_IPC_UNAUTHORIZED_PACKET;
        goto cleanup_mac;
    }

    /* create a payload buffer for holding the encrypted payload. */
    vccrypt_buffer_t payload;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(&payload, suite->alloc_opts, *size))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_mac;
    }

    /* read the payload. */
    if (*size != read(sock, payload.data, *size))
    {
        retval = AGENTD_ERROR_IPC_READ_BLOCK_FAILURE;
        goto cleanup_payload;
    }

    /* digest the packet. */
    if (VCCRYPT_STATUS_SUCCESS !=
            vccrypt_mac_digest(&mac, hbuffer.data, dheader_size) ||
        VCCRYPT_STATUS_SUCCESS !=
            vccrypt_mac_digest(&mac, payload.data, *size))
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_payload;
    }

    /* create a buffer to hold the digest. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t digest;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(
            &digest, suite->alloc_opts, suite->mac_short_opts.mac_size))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_payload;
    }

    /* finalize the mac. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_mac_finalize(&mac, &digest))
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_digest;
    }

    /* compare the digest against the mac in the packet. */
    if (0 !=
        crypto_memcmp(
            digest.data, header + sizeof(type) + sizeof(nsize),
            digest.size))
    {
        retval = AGENTD_ERROR_IPC_UNAUTHORIZED_PACKET;
        goto cleanup_digest;
    }

    /* the payload has been authenticated.  create output buffer. */
    *val = malloc(*size);
    if (NULL == *val)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_digest;
    }

    /* continue decryption in the payload. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_stream_continue_decryption(
            &stream, &iv, sizeof(iv), offset))
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_val;
    }

    /* reset the offset. */
    offset = 0;

    /* decrypt the payload. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_stream_decrypt(
            &stream, payload.data, *size, *val, &offset))
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_val;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_digest;

cleanup_val:
    memset(*val, 0, *size);
    free(*val);
    *val = NULL;

cleanup_digest:
    dispose((disposable_t*)&digest);

cleanup_payload:
    dispose((disposable_t*)&payload);

cleanup_mac:
    dispose((disposable_t*)&mac);

cleanup_stream:
    dispose((disposable_t*)&stream);

cleanup_dhbuffer:
    dispose((disposable_t*)&dhbuffer);

cleanup_hbuffer:
    dispose((disposable_t*)&hbuffer);

done:
    return retval;
}
