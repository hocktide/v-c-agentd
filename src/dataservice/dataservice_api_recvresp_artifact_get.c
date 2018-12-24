/**
 * \file dataservice/dataservice_api_recvresp_artifact_get.c
 *
 * \brief Read the response from the artifact get call.
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
 * \brief Receive a response from the get artifact query.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 * \param record        Pointer to the record to be updated with data from this
 *                      artifact record.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.
 *
 * A status code of 1 represents a "not found" error code.  In future versions
 * of this API, this will be updated to a enumerated value.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_artifact_get(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status,
    data_artifact_record_t* record)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != offset);
    MODEL_ASSERT(NULL != status);
    MODEL_ASSERT(NULL != record);

    /* | Artifact get response packet.                                      | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATA                                                | SIZE         | */
    /* | --------------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_APP_ARTIFACT_READ            |  4 bytes     | */
    /* | offset                                              |  4 bytes     | */
    /* | status                                              |  4 bytes     | */
    /* | record:                                             | 68 bytes     | */
    /* |    key                                              | 16 bytes     | */
    /* |    txn_first                                        | 16 bytes     | */
    /* |    txn_latest                                       | 16 bytes     | */
    /* |    net_height_first                                 |  8 bytes     | */
    /* |    net_height_latest                                |  8 bytes     | */
    /* |    net_state_latest                                 |  4 bytes     | */
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

    /* the size should be equal to the size we expect. */
    uint32_t response_packet_size =
        /* size of the API method. */
        sizeof(uint32_t) +
        /* size of the offset. */
        sizeof(uint32_t) +
        /* size of the status. */
        sizeof(uint32_t) +
        /* size of the payload. */
        68U;
    if (size != response_packet_size)
    {
        retval = 1;
        goto cleanup_val;
    }

    /* verify that the method code is the code we expect. */
    uint32_t code = ntohl(val[0]);
    if (DATASERVICE_API_METHOD_APP_ARTIFACT_READ != code)
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

    /* adjust the data size. */
    dat_size -= 3 * sizeof(uint32_t);

    /* if the record size is invalid, return an error code. */
    if (dat_size != 68U)
    {
        retval = 3;
        goto done;
    }

    /* get the raw data. */
    const uint8_t* bval = (const uint8_t*)(val + 3);

    /* clear the record. */
    memset(record, 0, sizeof(data_artifact_record_t));

    /* copy the key. */
    memcpy(record->key, bval, sizeof(record->key));

    /* copy the prev. */
    memcpy(record->txn_first, bval + 16, sizeof(record->txn_first));

    /* copy the next. */
    memcpy(record->txn_latest, bval + 32, sizeof(record->txn_latest));

    /* copy the first height. */
    memcpy(&record->net_height_first, bval + 48,
        sizeof(record->net_height_first));

    /* copy the latest height. */
    memcpy(&record->net_height_latest, bval + 56,
        sizeof(record->net_height_latest));

    /* copy the latest state. */
    memcpy(&record->net_state_latest, bval + 64,
        sizeof(record->net_state_latest));

    /* success. */
    retval = 0;

    /* fall-through. */

cleanup_val:
    memset(val, 0, size);
    free(val);

done:
    return retval;
}
