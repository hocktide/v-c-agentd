/**
 * \file dataservice/dataservice_api_sendreq_global_settings_set.c
 *
 * \brief Set a global setting using a 64-bit key.
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
 * \brief Set a global setting using a 64-bit key.
 *
 * \param sock          The socket on which this request is made.
 * \param child         The child index used for this operation.
 * \param key           The global key to set.
 * \param val           Buffer holding the value to set for this key.
 * \param val_size      The size of this key.
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
int dataservice_api_sendreq_global_settings_set(
    ipc_socket_context_t* sock, uint32_t child, uint64_t key, const void* val,
    uint32_t val_size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != val);

    /* | Global Settings set packet.                                    | */
    /* | ----------------------------------------------- | ------------ | */
    /* | DATA                                            | SIZE         | */
    /* | ----------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE | 4 bytes      | */
    /* | child_context_index                             | 4 bytes      | */
    /* | key                                             | 8 bytes      | */
    /* | value                                           | n - 16 bytes | */
    /* | ----------------------------------------------- | ------------ | */

    /* allocate a structure large enough for writing this request. */
    size_t reqbuflen = 2 * sizeof(uint32_t) + sizeof(uint64_t) + val_size;
    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE);
    memcpy(reqbuf, &req, sizeof(req));

    /* copy the child context index parameter to the buffer. */
    uint32_t nchild = htonl(child);
    memcpy(reqbuf + sizeof(req), &nchild, sizeof(nchild));

    /* copy the key to the buffer. */
    uint64_t nkey = htonll(key);
    memcpy(reqbuf + sizeof(req) + sizeof(nchild), &nkey, sizeof(nkey));

    /* copy the value to the buffer. */
    memcpy(reqbuf + sizeof(req) + sizeof(nchild) + sizeof(nkey), val, val_size);

    /* the request packet consists of the command, index, key, and value. */
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
