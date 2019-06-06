/**
 * \file ipc/ipc_read_authed_data_noblock.c
 *
 * \brief Non-blocking read of an authenticated data packet value.
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

#include "ipc_internal.h"

/**
 * \brief Read an authenticated data packet from a non-blocking socket.
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
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if the the operation was halted because
 *        it would block this thread.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if an unexpected data type
 *        was encountered when attempting to read this value.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if an unexpected data size
 *        was encountered when attempting to read this value.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error condition while executing.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE if draining the read buffer
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE if removing the payload
 *        from the read buffer failed.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_CRYPTO_SUITE if the crypto suite is
 *        invalid.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_SECRET if the secret key is invalid.
 *      - AGENTD_ERROR_IPC_AUTHENTICATION_FAILURE if the packet could not be
 *        authenticated.
 */
int ipc_read_authed_data_noblock(
    ipc_socket_context_t* sock, uint64_t iv, void** val, uint32_t* size,
    vccrypt_suite_options_t* suite, vccrypt_buffer_t* secret)
{
    int retval = 0;
    uint8_t type = 0U;
    uint32_t nsize = 0U;
    uint8_t* dheader = NULL;
    uint8_t* header = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != val);
    MODEL_ASSERT(NULL != size);
    MODEL_ASSERT(NULL != suite);
    MODEL_ASSERT(NULL != secret);

    /* get the socket details. */
    ipc_socket_impl_t* sock_impl = (ipc_socket_impl_t*)sock->impl;

    /* we need at least this many bytes to see the header. */
    ssize_t header_sz =
        sizeof(uint8_t) + sizeof(uint32_t) + suite->mac_short_opts.mac_size;

    /* get the size of the buffer. */
    ssize_t buffer_size = (ssize_t)evbuffer_get_length(sock_impl->readbuf);

    /* do we need to read more bytes for the header? */
    if (buffer_size < header_sz)
    {
        ssize_t needed_bytes = header_sz - buffer_size;

        /* read at least header_sz bytes into our buffer. */
        retval = evbuffer_read(sock_impl->readbuf, sock->fd, needed_bytes);
        if (retval < 0)
        {
            retval = AGENTD_ERROR_IPC_EVBUFFER_READ_FAILURE;
            goto done;
        }

        /* if there aren't enough bytes, return. */
        if (retval < needed_bytes)
        {
            retval = AGENTD_ERROR_IPC_WOULD_BLOCK;
            goto done;
        }
    }

    /* attempt to allocate space for the decrypted header. */
    size_t dheader_sz =
        sizeof(type) + sizeof(nsize);
    vccrypt_buffer_t dbuffer;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(&dbuffer, suite->alloc_opts, dheader_sz))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set up pointers for convenience. */
    dheader = (uint8_t*)dbuffer.data;

    /* get the encrypted header data. */
    header = (uint8_t*)evbuffer_pullup(sock_impl->readbuf, header_sz);
    if (NULL == header)
    {
        retval = AGENTD_ERROR_IPC_WOULD_BLOCK;
        goto cleanup_dbuffer;
    }

    /* set up the stream cipher. */
    vccrypt_stream_context_t stream;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_suite_stream_init(suite, &stream, secret))
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_dbuffer;
    }

    /* set up MAC. */
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
            &stream, header, dheader_sz, dbuffer.data, &offset))
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

    /* compute the total packet size. */
    ssize_t packet_size = header_sz + *size;

    /* get the size of the buffer. */
    buffer_size = evbuffer_get_length(sock_impl->readbuf);

    /* do we need to read more bytes for the payload? */
    if (buffer_size < packet_size)
    {
        ssize_t needed_bytes = packet_size - buffer_size;

        /* read any needed bytes into our buffer. */
        retval = evbuffer_read(sock_impl->readbuf, sock->fd, needed_bytes);
        if (retval < 0)
        {
            retval = AGENTD_ERROR_IPC_EVBUFFER_READ_FAILURE;
            goto cleanup_mac;
        }

        /* if there aren't enough bytes, return. */
        if (retval < needed_bytes)
        {
            retval = AGENTD_ERROR_IPC_WOULD_BLOCK;
            goto cleanup_mac;
        }
    }

    /* pull up the entire packet. */
    header = (uint8_t*)evbuffer_pullup(sock_impl->readbuf, packet_size);
    if (NULL == header)
    {
        retval = AGENTD_ERROR_IPC_WOULD_BLOCK;
        goto cleanup_mac;
    }

    /* digest the packet. */
    if (VCCRYPT_STATUS_SUCCESS !=
            vccrypt_mac_digest(&mac, header, dheader_sz) ||
        VCCRYPT_STATUS_SUCCESS !=
            vccrypt_mac_digest(&mac, header + header_sz, *size))
    {
        retval = AGENTD_ERROR_IPC_CRYPTO_FAILURE;
        goto cleanup_mac;
    }

    /* create a buffer to hold the digest. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t digest;
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_buffer_init(
            &digest, suite->alloc_opts, suite->mac_short_opts.mac_size))
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_mac;
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
            digest.data, header + dheader_sz,
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
            &stream, header + header_sz, *size, *val, &offset))
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

cleanup_mac:
    dispose((disposable_t*)&mac);

cleanup_stream:
    dispose((disposable_t*)&stream);

cleanup_dbuffer:
    dispose((disposable_t*)&dbuffer);

done:
    return retval;
}
