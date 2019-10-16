/**
 * \file agentd/dataservice/async_api.h
 *
 * \brief Asynchronous API for the data service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_DATASERVICE_ASYNC_API_HEADER_GUARD
#define AGENTD_DATASERVICE_ASYNC_API_HEADER_GUARD

#include <agentd/dataservice.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Response Payload Header.
 */
typedef struct dataservice_response_header
{
    disposable_t hdr;
    uint32_t method_code;
    uint32_t offset;
    uint32_t status;
    size_t payload_size;
} dataservice_response_header_t;

/**
 * \brief Root Context Init Response.
 */
typedef struct dataservice_response_root_context_init
{
    dataservice_response_header_t hdr;
} dataservice_response_root_context_init_t;

/**
 * \brief Root Context Reduce Caps Response.
 */
typedef struct dataservice_response_root_context_reduce_caps
{
    dataservice_response_header_t hdr;
} dataservice_response_root_context_reduce_caps_t;

/**
 * \brief The memset disposer simply clears the data structure when disposed.
 *
 * \param disposable    The disposable to clear.
 */
void dataservice_decode_response_memset_disposer(void* disposable);

/**
 * \brief Decode a root context init response into its constituent pieces.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_root_context_init(
    const void* resp, size_t size,
    dataservice_response_root_context_init_t* dresp);

/**
 * \brief Decode a response from the root context reduce capabilities call.
 *
 * \param resp          The response payload to parse.
 * \param size          The size of this response payload.
 * \param dresp         The decoded response structure into which this response
 *                      is decoded.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE if the response
 *        packet payload size is incorrect.
 *      - AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER if one of the
 *        parameters to the function is invalid.
 */
int dataservice_decode_response_root_context_reduce_caps(
    const void* resp, size_t size,
    dataservice_response_root_context_reduce_caps_t* dresp);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_ASYNC_API_HEADER_GUARD*/
