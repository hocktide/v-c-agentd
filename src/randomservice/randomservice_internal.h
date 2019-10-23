/**
 * \file randomservice/randomservice_internal.h
 *
 * \brief Internal header for the random service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_RANDOMSERVICE_INTERNAL_HEADER_GUARD
#define AGENTD_RANDOMSERVICE_INTERNAL_HEADER_GUARD

#include <agentd/randomservice.h>
#include <agentd/randomservice/private/randomservice.h>
#include <agentd/ipc.h>
#include <event.h>
#include <lmdb.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Create a randomservice instance.
 *
 * \param random        File descriptor pointing to /dev/random.
 *
 * \returns a properly created randomservice instance, or NULL on failure.
 */
randomservice_root_context_t* randomservice_instance_create(int random);

/**
 * \brief Decode and dispatch requests received by the random service.
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
 *      - a non-zero status indicating failure.
 */
int randomservice_decode_and_dispatch(
    randomservice_root_context_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size);

/**
 * \brief Write a status response to the socket.
 *
 * Returns 0 on success or non-fatal error.  If a non-zero error message is
 * returned, then a fatal error has occurred that should not be recovered from.
 *
 * \param sock          The socket on which the request was received and the
 *                      response is to be written.
 * \param method        The API method of this request.
 * \param offset        The offset for the child context.
 * \param status        The status returned from this API method.
 * \param data          Additional payload data for this call.  May be NULL.
 * \param data_size     The size of this additional payload data.  Must be 0 if
 *                      data is NULL.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_RANDOMSERVICE_IPC_WRITE_DATA_FAILURE if data could not be
 *        written to the client socket.
 */
int randomservice_decode_and_dispatch_write_status(
    ipc_socket_context_t* sock, uint32_t method, uint32_t offset,
    uint32_t status, void* data, size_t data_size);

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
    randomservice_root_context_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_RANDOMSERVICE_INTERNAL_HEADER_GUARD*/
