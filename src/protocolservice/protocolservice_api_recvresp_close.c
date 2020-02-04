/**
 * \file protocolservice/protocolservice_api_recvresp_close.c
 *
 * \brief Receive a response from the close call.
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
 * \brief Receive a response from the explicit close connection request.
 *
 * \param sock                      The socket from which this response is read.
 * \param suite                     The crypto suite to use to verify this
 *                                  response.
 * \param server_iv                 Pointer to the server IV, updated by this
 *                                  call.
 * \param shared_secret             The shared secret key for this response.
 *
 * After the response has been successfully received, this socket can be closed.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int protocolservice_api_recvresp_close(
    int sock, vccrypt_suite_options_t* suite, uint64_t* server_iv,
    const vccrypt_buffer_t* shared_secret)
{
    int retval;

    /* parameter sanity checking. */
    MODEL_ASSERT(NULL != suite);
    MODEL_ASSERT(NULL != server_iv);
    MODEL_ASSERT(NULL != shared_secret);

    /* read the response from the server. */
    /* TODO - fix constness in ipc method for shared secret. */
    uint32_t* val;
    uint32_t size;
    retval =
        ipc_read_authed_data_block(
            sock, *server_iv, (void**)&val, &size, suite,
            (vccrypt_buffer_t*)shared_secret);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* update the server_iv on sucessful read. */
    *server_iv += 1;

    /* verify that the response is the correct size. */
    if (size < 3 * sizeof(uint32_t))
    {
        retval = AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE;
        goto cleanup_val;
    }

    /* verify the request id. */
    if (UNAUTH_PROTOCOL_REQ_ID_CLOSE != ntohl(val[0]))
    {
        retval = AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE;
        goto cleanup_val;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_val;

cleanup_val:
    memset(val, 0, size);
    free(val);

done:
    return retval;
}
