/**
 * \file canonization/canonization_api_sendreq_configure.c
 *
 * \brief Configure the canonization service.
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#include <arpa/inet.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Configure the canonization service.
 *
 * \param sock          The socket on which this request is made.
 * \param conf          The config data for this agentd instance.
 *
 * This must be the first API call on the canonization control socket.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory condition.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_WRITE_DATA_FAILURE if an error
 *        occurred when writing to the socket.
 */
int canonization_api_sendreq_configure(
    int sock, const agent_config_t* conf)
{
    /* | Canonization service configure request packet.               | */
    /* | --------------------------------------------- | ------------ | */
    /* | DATA                                          | SIZE         | */
    /* | --------------------------------------------- | ------------ | */
    /* | CANONIZATIONSERVICE_API_METHOD_CONFIGURE      |  4 bytes     | */
    /* | sleep milliseconds (uint64_t)                 |  8 bytes     | */
    /* | max transactions per block (uint64_t)         |  8 bytes     | */
    /* | --------------------------------------------- | ------------ | */
    /* | total                                         | 20 bytes     | */
    /* | --------------------------------------------- | ------------ | */

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != conf);
    MODEL_ASSERT(conf->block_max_milliseconds_set);
    MODEL_ASSERT(conf->block_max_transactions_set);

    /* runtime parameter sanity check. */
    if (NULL == conf || !conf->block_max_milliseconds_set ||
        !conf->block_max_transactions_set)
    {
        return AGENTD_ERROR_CANONIZATIONSERVICE_BAD_PARAMETER;
    }

    /* compute the request buffer length. */
    size_t reqbuflen =
        /* method. */
        sizeof(uint32_t) +
        /* sleep milliseconds. */
        sizeof(uint64_t) +
        /* max transactions per block. */
        sizeof(uint64_t);

    /* allocate the request buffer. */
    uint8_t* reqbuf = (uint8_t*)malloc(reqbuflen);
    if (NULL == reqbuf)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* copy the request ID to the buffer. */
    uint32_t req = htonl(CANONIZATIONSERVICE_API_METHOD_CONFIGURE);
    memcpy(reqbuf, &req, sizeof(req));

    /* copy the sleep milliseconds parameter to the buffer. */
    uint64_t sleep_milliseconds = htonll(conf->block_max_milliseconds);
    memcpy(
        reqbuf + sizeof(uint32_t), &sleep_milliseconds,
        sizeof(sleep_milliseconds));

    /* copy the max transactions per block parameter to the buffer. */
    uint64_t max_txns = htonll(conf->block_max_transactions);
    memcpy(
        reqbuf + sizeof(uint32_t) + sizeof(uint64_t), &max_txns,
        sizeof(max_txns));

    /* write the request packet to the control socket. */
    int retval = ipc_write_data_block(sock, reqbuf, reqbuflen);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        retval = AGENTD_ERROR_CANONIZATIONSERVICE_IPC_WRITE_DATA_FAILURE;
    }

    /* clean up memory. */
    memset(reqbuf, 0, reqbuflen);
    free(reqbuf);

    /* return the status of the socket write to the caller. */
    return retval;
}
