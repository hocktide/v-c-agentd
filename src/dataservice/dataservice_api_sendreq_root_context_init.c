/**
 * \file dataservice/dataservice_api_sendreq_root_context_init.c
 *
 * \brief Request the creation of a root data service context.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Request the creation of a root data service context.
 *
 * \param sock          The socket on which this request is made.
 * \param datadir       The data directory to open.
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
int dataservice_api_sendreq_root_context_init(
    ipc_socket_context_t* sock, const char* datadir)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != datadir);

    /* | Root context init request packet.                            | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATA                                          | SIZE         | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE | 4 bytes      | */
    /* | datadir                                       | n - 4 bytes  | */
    /* | --------------------------------------------- | ------------ | */

    /* compute the length of the datadir parameter. */
    size_t datadirlen = strlen(datadir);

    /* allocate a structure large enough for writing this request. */
    size_t reqbuflen = datadirlen + sizeof(uint32_t);
    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE);
    memcpy(reqbuf, &req, sizeof(req));

    /* copy the datadir parameter to this buffer. */
    memcpy(reqbuf + sizeof(req), datadir, datadirlen);

    /* the request packet consists of the data directory only. */
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
