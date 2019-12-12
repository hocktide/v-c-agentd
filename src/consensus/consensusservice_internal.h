/**
 * \file consensus/consensusservice_internal.h
 *
 * \brief Internal header for the consensus service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_CONSENSUSSERVICE_INTERNAL_HEADER_GUARD
#define AGENTD_CONSENSUSSERVICE_INTERNAL_HEADER_GUARD

#include <agentd/ipc.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

typedef struct consensusservice_instance
{
    disposable_t hdr;
    bool configured;
    bool running;
    bool force_exit;
    int64_t block_max_seconds;
    int64_t block_max_transactions;
    ipc_event_loop_context_t* loop_context;
} consensusservice_instance_t;

/**
 * \brief Create the consensus service instance.
 *
 * \returns a properly created consensus service instance, or NULL on failure.
 */
consensusservice_instance_t* consensusservice_instance_create();

/**
 * \brief Decode and dispatch requests received by the consensus service on the
 * control socket.
 *
 * Returns \ref AGENTD_STATUS_SUCCESS on success or non-fatal error.  If a
 * non-zero error message is returned, then a fatal error has occurred that
 * should not be recovered from. Any additional information on the socket is
 * suspect.
 *
 * \param instance      The instance on which the dispatch occurs.
 * \param sock          The socket on which the request was received and the
 *                      response is to be written.
 * \param req           The request to be decoded and dispatched.
 * \param size          The size of the request.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_REQUEST_PACKET_INVALID_SIZE if the
 *        request packet size is invalid.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_IPC_WRITE_DATA_FAILURE if data could not
 *        be written to the client socket.
 */
int consensus_service_decode_and_dispatch_control_command(
    consensusservice_instance_t* instance, ipc_socket_context_t* sock,
    const void* req, size_t size);

/**
 * \brief Decode and dispatch a configure request.
 *
 * Returns \ref AGENTD_STATUS_SUCCESS on success or non-fatal error.  If a
 * non-zero error message is returned, then a fatal error has occurred that
 * should not be recovered from. Any additional information on the socket is
 * suspect.
 *
 * \param instance      The instance on which the dispatch occurs.
 * \param sock          The socket on which the request was received and the
 *                      response is to be written.
 * \param req           The request to be decoded and dispatched.
 * \param size          The size of the request.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_IPC_WRITE_DATA_FAILURE if data could not
 *        be written to the client socket.
 */
int consensus_service_decode_and_dispatch_control_command_configure(
    consensusservice_instance_t* instance, ipc_socket_context_t* sock,
    const void* req, size_t size);

/**
 * \brief Decode and dispatch a start request.
 *
 * Returns \ref AGENTD_STATUS_SUCCESS on success or non-fatal error.  If a
 * non-zero error message is returned, then a fatal error has occurred that
 * should not be recovered from. Any additional information on the socket is
 * suspect.
 *
 * \param instance      The instance on which the dispatch occurs.
 * \param sock          The socket on which the request was received and the
 *                      response is to be written.
 * \param req           The request to be decoded and dispatched.
 * \param size          The size of the request.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_CONSENSUSSERVICE_IPC_WRITE_DATA_FAILURE if data could not
 *        be written to the client socket.
 */
int consensus_service_decode_and_dispatch_control_command_start(
    consensusservice_instance_t* instance, ipc_socket_context_t* sock,
    const void* req, size_t size);

/**
 * \brief Write a status response to the socket.
 *
 * Returns \ref AGENTD_STATUS_SUCCESS on success or non-fatal error.  If a
 * non-zero error message is returned, then a fatal error has occurred that
 * should not be recovered from.
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
 *      - AGENTD_ERROR_CONSENSUSSERVICE_IPC_WRITE_DATA_FAILURE if data could not
 *        be written to the client socket.
 */
int consensus_service_decode_and_dispatch_write_status(
    ipc_socket_context_t* sock, uint32_t method, uint32_t offset,
    uint32_t status, void* data, size_t data_size);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_CONSENSUSSERVICE_INTERNAL_HEADER_GUARD*/
