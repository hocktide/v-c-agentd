/**
 * \file dataservice/dataservice_api_sendreq_transaction_submit.c
 *
 * \brief Submit a transaction to the transaction queue.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Submit a transaction to the transaction queue.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index used for this operation.
 * \param txn_id        The transaction UUID bytes for this transaction.
 * \param artifact_id   The artifact UUID bytes for this transaction.
 * \param val           Buffer holding the raw bytes for the transaction cert.
 * \param val_size      The size of this transaction cert.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_transaction_submit(
    ipc_socket_context_t* sock, uint32_t child, const uint8_t* txn_id,
    const uint8_t* artifact_id, const void* val, uint32_t val_size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != txn_id);
    MODEL_ASSERT(NULL != artifact_id);
    MODEL_ASSERT(NULL != val);

    /* | Transaction Submit Packet.                                     | */
    /* | ------------------------------------------------ | ------------ | */
    /* | DATA                                             | SIZE         | */
    /* | ------------------------------------------------ | ------------ | */
    /* | DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT | 4 bytes      | */
    /* | child_context_index                              | 4 bytes      | */
    /* | txn_id                                           | 16 bytes     | */
    /* | artifact_id                                      | 16 bytes     | */
    /* | txn_cert                                         | n - 40 bytes | */
    /* | ------------------------------------------------ | ------------ | */

    /* allocate a structure large enough for writing this request. */
    size_t reqbuflen = 2 * sizeof(uint32_t) + 2 * 16 + val_size;
    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return 1;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT);
    memcpy(reqbuf, &req, sizeof(req));

    /* copy the child context index parameter to the buffer. */
    uint32_t nchild = htonl(child);
    memcpy(reqbuf + sizeof(req), &nchild, sizeof(nchild));

    /* copy the transaction id to the buffer. */
    memcpy(reqbuf + sizeof(req) + sizeof(nchild), txn_id, 16);

    /* copy the artifact id to the buffer. */
    memcpy(reqbuf + sizeof(req) + sizeof(nchild) + 16, artifact_id, 16);

    /* copy the value to the buffer. */
    memcpy(reqbuf + sizeof(req) + sizeof(nchild) + 32, val, val_size);

    /* the request packet consists of the command, index, txn_id, artifact_id,
     * and value. */
    int retval = ipc_write_data_noblock(sock, reqbuf, reqbuflen);

    /* clean up memory. */
    memset(reqbuf, 0, reqbuflen);
    free(reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
