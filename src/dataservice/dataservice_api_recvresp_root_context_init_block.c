/**
 * \file dataservice/dataservice_api_recvresp_root_context_init_block.c
 *
 * \brief Read the response from the root context init api method using a
 * blocking socket.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Receive a response from the root context init api method call.
 *
 * \param sock          The socket on which this request is made.
 * \param offset        The child context offset for this response.
 * \param status        This value is updated with the status code returned from
 *                      the request.
 *
 * On a successful return from this function, the status is updated with the
 * status code from the API request.  This status should be checked.  A zero
 * status indicates success, and a non-zero status indicates failure.
 *
 * \returns 0 if the response was read successfully, IPC_ERROR_CODE_WOULD_BLOCK
 * if the response cannot yet be read, and non-zero if the response could not be
 * successfully read.
 */
int dataservice_api_recvresp_root_context_init_block(
    int sock, uint32_t* offset, uint32_t* status)
{
    int retval = 0;
    uint32_t* val;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != status);

    /* | Root context init response packet.                           | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATA                                          | SIZE         | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE | 4 bytes      | */
    /* | offset                                        | 4 bytes      | */
    /* | status                                        | 4 bytes      | */
    /* | --------------------------------------------- | ------------ | */

    /* compute the data packet size. */
    uint32_t data_size = 3 * sizeof(uint32_t);

    /* read a data packet from the socket. */
    uint32_t newsize = data_size;
    retval = ipc_read_data_block(sock, (void**)&val, &newsize);
    if (0 != retval)
    {
        retval = 1;
        goto done;
    }

    /* compare the sizes. */
    if (data_size != newsize)
    {
        retval = 2;
        goto cleanup_val;
    }

    /* verify that the method code is the code we expect. */
    uint32_t code = ntohl(val[0]);
    if (DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE != code)
    {
        retval = 3;
        goto cleanup_val;
    }

    /* get the offset. */
    *offset = ntohl(val[1]);

    /* get the status code. */
    *status = ntohl(val[2]);

    /* success. */
    retval = 0;

    /* fall-through. */

cleanup_val:
    memset(val, 0, data_size);
    free(val);

done:
    return retval;
}
