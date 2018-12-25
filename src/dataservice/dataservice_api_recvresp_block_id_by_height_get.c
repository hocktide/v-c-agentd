/**
 * \file dataservice/dataservice_api_recvresp_block_id_by_height_get.c
 *
 * \brief Read the response from the block get by height call.
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
 * \brief Receive a response from the get block id by height query.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 * \param block_id      Pointer to the 16 byte array to receive the block id.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.  On
 * success, the block id array is updated with the UUID for the block associated
 * with the height provided when the original request was sent.
 *
 * A status code of 1 represents a "not found" error code.  In future versions
 * of this API, this will be updated to a enumerated value.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_block_id_by_height_get(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status,
    uint8_t* block_id)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != status);
    MODEL_ASSERT(NULL != block_id);

    /* | Block get by height response packet.                               | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATA                                                | SIZE         | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ  |  4 bytes     | */
    /* | offset                                              |  4 bytes     | */
    /* | status                                              |  4 bytes     | */
    /* | block_id                                            | 16 bytes     | */
    /* | --------------------------------------------------- | ------------ | */

    /* read a data packet from the socket. */
    uint32_t* val = NULL;
    uint32_t size = 0U;
    retval = ipc_read_data_noblock(sock, (void**)&val, &size);
    if (0 != retval)
    {
        goto done;
    }

    /* the size should be equal to the size we expect. */
    uint32_t response_packet_size =
        /* size of the API method. */
        sizeof(uint32_t) +
        /* size of the offset. */
        sizeof(uint32_t) +
        /* size of the status. */
        sizeof(uint32_t) +
        /* size of the block id. */
        16;
    if (size != response_packet_size)
    {
        retval = 1;
        goto cleanup_val;
    }

    /* verify that the method code is the code we expect. */
    uint32_t code = ntohl(val[0]);
    if (DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ != code)
    {
        retval = 2;
        goto cleanup_val;
    }

    /* get the offset. */
    *offset = ntohl(val[1]);

    /* get the status code. */
    *status = ntohl(val[2]);
    if (*status != 0)
    {
        retval = 0;
        goto done;
    }

    /* get the raw data. */
    const uint8_t* bval = (const uint8_t*)(val + 3);

    /* copy the block id. */
    memcpy(block_id, bval, 16);

    /* success. */
    retval = 0;

    /* fall-through. */

cleanup_val:
    memset(val, 0, size);
    free(val);

done:
    return retval;
}
