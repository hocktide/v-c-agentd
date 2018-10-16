/**
 * \file dataservice/dataservice_api_recvresp_transaction_get.c
 *
 * \brief Read the response from the transaction get call.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Receive a response from the get transaction query.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 * \param node          Pointer to the node to be updated with data from this
 *                      node in the queue.
 * \param data          This pointer is updated with the data received from the
 *                      response.  The caller owns this buffer and it must be
 *                      freed when no longer needed.
 * \param data_size     Pointer to the size of the data buffer.  On successful
 *                      execution, this size is updated with the size of the
 *                      data allocated for this buffer.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.  On
 * success, the data pointer and size are both updated to reflect the data read
 * from the query.  This is a dynamically allocated buffer that must be freed by
 * the caller.
 *
 * A status code of 2 represents a "not found" error code.  In future versions
 * of this API, this will be updated to a enumerated value.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_transaction_get(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status,
    data_transaction_node_t* node, void** data, size_t* data_size)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != status);
    MODEL_ASSERT(NULL != data);
    MODEL_ASSERT(NULL != data_size);

    /* | Transaction get first response packet.                             | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATA                                                | SIZE         | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ      |  4 bytes     | */
    /* | offset                                              |  4 bytes     | */
    /* | status                                              |  4 bytes     | */
    /* | node:                                               | 64 bytes     | */
    /* |    key                                              | 16 bytes     | */
    /* |    prev                                             | 16 bytes     | */
    /* |    next                                             | 16 bytes     | */
    /* |    artifact_id                                      | 16 bytes     | */
    /* | data                                                | n - 76 bytes | */
    /* | --------------------------------------------------- | ------------ | */

    /* read a data packet from the socket. */
    uint32_t* val = NULL;
    uint32_t size = 0U;
    retval = ipc_read_data_noblock(sock, (void**)&val, &size);
    if (0 != retval)
    {
        goto done;
    }

    /* set up data size for later. */
    uint32_t dat_size = size;

    /* the size should be greater than or equal to the size we respect. */
    uint32_t response_packet_size =
        /* size of the API method. */
        sizeof(uint32_t) +
        /* size of the offset. */
        sizeof(uint32_t) +
        /* size of the status. */
        sizeof(uint32_t);
    if (size < response_packet_size)
    {
        retval = 1;
        goto cleanup_val;
    }

    /* verify that the method code is the code we expect. */
    uint32_t code = ntohl(val[0]);
    if (DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ != code)
    {
        retval = 2;
        goto cleanup_val;
    }

    /* get the offset. */
    *offset = ntohl(val[1]);

    /* get the status code. */
    *status = ntohl(val[2]);
    if (*status != 0 || dat_size < 4 * 16)
    {
        retval = 3;
        goto done;
    }

    /* get the raw data. */
    const uint8_t* bval = (const uint8_t*)(val + 3);
    /* update data size. */
    dat_size -= 3 * sizeof(uint32_t) + 4 * 16;

    /* process the node data if the node is specified. */
    if (NULL != node)
    {
        /* clear the node. */
        memset(node, 0, sizeof(data_transaction_node_t));

        /* copy the key. */
        memcpy(node->key, bval, sizeof(node->key));

        /* copy the prev. */
        memcpy(node->prev, bval + 16, sizeof(node->prev));

        /* copy the next. */
        memcpy(node->next, bval + 32, sizeof(node->next));

        /* copy the artifact_id. */
        memcpy(node->artifact_id, bval + 48, sizeof(node->artifact_id));

        /* set the size. */
        node->net_txn_cert_size = htonll(dat_size);
    }

    /* get to the location of the data. */
    bval += 4 * 16;

    /* allocate memory for the data. */
    *data = malloc(dat_size);
    if (NULL == *data)
    {
        retval = 4;
        goto cleanup_val;
    }

    /* copy data. */
    memcpy(*data, bval, dat_size);
    *data_size = dat_size;

    /* success. */
    retval = 0;

    /* fall-through. */

cleanup_val:
    memset(val, 0, size);
    free(val);

done:
    return retval;
}
