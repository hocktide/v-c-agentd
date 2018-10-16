/**
 * \file dataservice/dataservice_api_sendreq_transaction_get.c
 *
 * \brief Get a transaction by id from the transaction queue.
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
 * \brief Get a transaction from the transaction queue by ID.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index used for the query.
 * \param txn_id        The transaction UUID of the transaction to retrieve.
 *
 * \returns 0 if the request was successfully written to the socket, and
 * non-zero otherwise.
 */
int dataservice_api_sendreq_transaction_get(
    ipc_socket_context_t* sock, uint32_t child, const uint8_t* txn_id)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);

    /* | Transaction Queue Get packet.                                        */
    /* | ---------------------------------------------------- | ----------- | */
    /* | DATA                                                 | SIZE        | */
    /* | ---------------------------------------------------- | ----------- | */
    /* | DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ       |  4 bytes    | */
    /* | child_context_index                                  |  4 bytes    | */
    /* | transaction UUID.                                    | 16 bytes    | */
    /* | ---------------------------------------------------- | ----------- | */

    /* allocate a structure large enough for writing this request. */
    size_t reqbuflen = 2 * sizeof(uint32_t) + 16;
    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return 1;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ);
    memcpy(reqbuf, &req, sizeof(req));

    /* copy the child context index parameter to the buffer. */
    uint32_t nchild = htonl(child);
    memcpy(reqbuf + sizeof(req), &nchild, sizeof(nchild));

    /* copy the transaction id to the buffer. */
    memcpy(reqbuf + 2 * sizeof(uint32_t), txn_id, 16);

    /* the request packet consists of the command, index, and transaction id. */
    int retval = ipc_write_data_noblock(sock, reqbuf, reqbuflen);

    /* clean up memory. */
    memset(reqbuf, 0, reqbuflen);
    free(reqbuf);

    /* return the status of this request write to the caller. */
    return retval;
}
