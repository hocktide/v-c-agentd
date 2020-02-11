/**
 * \file dataservice/dataservice_api_sendreq_transaction_promote.c
 *
 * \brief Promote a transaction by id from the transaction queue.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
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
 * \brief Promote a transaction from the transaction queue by ID.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index used for the query.
 * \param txn_id        The transaction UUID of the transaction to promote.
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
int dataservice_api_sendreq_transaction_promote(
    ipc_socket_context_t* sock, uint32_t child, const uint8_t* txn_id)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);

    /* | Transaction Queue Promote packet.                                    */
    /* | ---------------------------------------------------- | ----------- | */
    /* | DATA                                                 | SIZE        | */
    /* | ---------------------------------------------------- | ----------- | */
    /* | DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_PROMOTE    |  4 bytes    | */
    /* | child_context_index                                  |  4 bytes    | */
    /* | transaction UUID.                                    | 16 bytes    | */
    /* | ---------------------------------------------------- | ----------- | */

    /* allocate a structure large enough for writing this request. */
    size_t reqbuflen = 2 * sizeof(uint32_t) + 16;
    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_PROMOTE);
    memcpy(reqbuf, &req, sizeof(req));

    /* copy the child context index parameter to the buffer. */
    uint32_t nchild = htonl(child);
    memcpy(reqbuf + sizeof(req), &nchild, sizeof(nchild));

    /* copy the transaction id to the buffer. */
    memcpy(reqbuf + 2 * sizeof(uint32_t), txn_id, 16);

    /* the request packet consists of the command, index, and transaction id. */
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
