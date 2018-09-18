/**
 * \file dataservice/dataservice_api_recvresp_child_context_close.c
 *
 * \brief Read the response from the child context close call.
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
 * \brief Receive a response from the child context close API call.
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
int dataservice_api_recvresp_child_context_close(
    ipc_socket_context_t* sock, uint32_t* offset, uint32_t* status)
{
    int retval = 0;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != status);

    /* | Child context create response packet.                             | */
    /* | -------------------------------------------------- | ------------ | */
    /* | DATA                                               | SIZE         | */
    /* | -------------------------------------------------- | ------------ | */
    /* | DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE      | 4 bytes      | */
    /* | offset                                             | 4 bytes      | */
    /* | status                                             | 4 bytes      | */
    /* | -------------------------------------------------- | ------------ | */

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
        sizeof(uint32_t);
    if (size != response_packet_size)
    {
        retval = 1;
        goto cleanup_val;
    }

    /* verify that the method code is the code we expect. */
    uint32_t code = ntohl(val[0]);
    if (DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE != code)
    {
        retval = 2;
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
    memset(val, 0, size);
    free(val);

done:
    return retval;
}
