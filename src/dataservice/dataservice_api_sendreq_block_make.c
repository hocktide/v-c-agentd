/**
 * \file dataservice/dataservice_api_sendreq_block_make.c
 *
 * \brief Make a block from transactions in the transaction queue.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Make a block from transactions in the transaction queue.
 *
 * Caller submits a valid signed block containing the transactions to drop from
 * the transaction queue.  If this call is successful, then this block and those
 * transactions are canonized.
 *
 * \param sock              The socket on which this request is made.
 * \param child             The child index used for this operation.
 * \param txn_id            The block UUID bytes for this transaction.
 * \param block_cert        Buffer holding the raw bytes for the block cert.
 * \param block_cert_size   The size of this block cert.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if this write operation would block this
 *        thread.
 *      - AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE if an error occurred
 *        when writing to the socket.
 */
int dataservice_api_sendreq_block_make(
    ipc_socket_context_t* sock, uint32_t child, const uint8_t* block_id,
    const void* block_cert, uint32_t block_cert_size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != block_id);
    MODEL_ASSERT(NULL != block_cert);
    MODEL_ASSERT(block_cert_size > 0);

    /* | Block Make Packet.                                              | */
    /* | ------------------------------------------------ | ------------ | */
    /* | DATA                                             | SIZE         | */
    /* | ------------------------------------------------ | ------------ | */
    /* | DATASERVICE_API_METHOD_APP_BLOCK_WRITE           | 4 bytes      | */
    /* | child_context_index                              | 4 bytes      | */
    /* | block_id                                         | 16 bytes     | */
    /* | block_cert                                       | n - 40 bytes | */
    /* | ------------------------------------------------ | ------------ | */

    /* allocate a structure large enough for writing this request. */
    size_t reqbuflen = 2 * sizeof(uint32_t) + 1 * 16 + block_cert_size;
    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_APP_BLOCK_WRITE);
    memcpy(reqbuf, &req, sizeof(req));

    /* copy the child context index parameter to the buffer. */
    uint32_t nchild = htonl(child);
    memcpy(reqbuf + sizeof(req), &nchild, sizeof(nchild));

    /* copy the block id to the buffer. */
    memcpy(reqbuf + sizeof(req) + sizeof(nchild), block_id, 16);

    /* copy the value to the buffer. */
    memcpy(reqbuf + sizeof(req) + sizeof(nchild) + 16,
        block_cert, block_cert_size);

    /* the request packet consists of the command, index, block_id, and
     * block cert. */
    int retval = ipc_write_data_noblock(sock, reqbuf, reqbuflen);
    if (AGENTD_ERROR_IPC_WOULD_BLOCK != retval && AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_DATASERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up memory. */
    memset(reqbuf, 0, reqbuflen);
    free(reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
