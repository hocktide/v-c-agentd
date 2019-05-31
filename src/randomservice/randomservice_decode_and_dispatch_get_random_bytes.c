/**
 * \file randomservice/randomservice_decode_and_dispatch_get_random_bytes.c
 *
 * \brief Decode and dispatch a get random bytes request.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/private/randomservice.h>
#include <agentd/inet.h>
#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "randomservice_internal.h"

/**
 * \brief Decode and dispatch a get random bytes request.
 *
 * Returns 0 on success or non-fatal error.  If a non-zero error message is
 * returned, then a fatal error has occurred that should not be recovered from.
 * Any additional information on the socket is suspect.
 *
 * \param inst          The instance on which the dispatch occurs.
 * \param sock          The socket on which the request was received and the
 *                      response is to be written.
 * \param req           The request to be decoded and dispatched.
 * \param size          The size of the request.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - A non-zero fatal error.
 */
int randomservice_decode_and_dispatch_get_random_bytes(
    randomservice_root_context_t* inst,
    ipc_socket_context_t* sock, void* req, size_t size)
{
    int retval = 0;
    uint32_t offset = 0;
    uint32_t rndsize = 0;
    uint8_t buffer[1024];

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != req);

    /* make working with the request more convenient. */
    uint8_t* breq = (uint8_t*)req;

    /* clear the buffer before we do anything else. */
    memset(buffer, 0, sizeof(buffer));

    /* the payload size should be equal to the size of a 32-bit size and the
     * 32-bit offset. */
    if (size != sizeof(uint32_t) * 2)
    {
        retval = AGENTD_ERROR_RANDOMSERVICE_REQUEST_PACKET_INVALID_SIZE;
        goto done;
    }

    /* copy the offset. */
    memcpy(&offset, breq, sizeof(uint32_t));

    /* increment breq and decrement size. */
    breq += sizeof(uint32_t);
    size -= sizeof(uint32_t);

    /* decode the offset. */
    offset = ntohl(offset);

    /* copy the size. */
    memcpy(&rndsize, breq, sizeof(uint32_t));

    /* increment breq and decrement size. */
    breq += sizeof(uint32_t);
    size -= sizeof(uint32_t);

    /* decode the index. */
    rndsize = ntohl(rndsize);

    /* verify that this size is sane. */
    if (rndsize == 0 || rndsize > 1024)
    {
        rndsize = 0;
        retval = AGENTD_ERROR_RANDOMSERVICE_GET_RANDOM_BYTES_INVALID_SIZE;
        goto done;
    }

    /* read data from the random fd. */
    if (read(inst->random_fd, buffer, rndsize) < rndsize)
    {
        rndsize = 0;
        retval = AGENTD_ERROR_RANDOMSERVICE_GET_RANDOM_BYTES_READ_FAILED;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;

done:
    /* write the status to the caller. */
    retval =
        randomservice_decode_and_dispatch_write_status(
            sock, RANDOMSERVICE_API_METHOD_GET_RANDOM_BYTES, offset,
            (uint32_t)retval, buffer, rndsize);

    /* clear the buffer. */
    memset(buffer, 0, sizeof(buffer));

    return retval;
}
