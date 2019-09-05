/**
 * \file authservice/auth_service_private.h
 *
 * \brief Private auth service functions and data structures.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_AUTH_SERVICE_PRIVATE_HEADER_GUARD
#define AGENTD_AUTH_SERVICE_PRIVATE_HEADER_GUARD

#include <agentd/authservice.h>
#include <agentd/ipc.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/* Forward decl for auth service data structure. */
struct auth_service_instance;


/* Forward decl for an auth connection. */
struct auth_connection;


/**
 * \brief The auth service instance.
 */
typedef struct auth_service_instance auth_service_instance_t;


/**
 * \brief Auth service instance.
 */
struct auth_service_instance
{
    disposable_t hdr;
    ipc_socket_context_t auth;
    bool auth_service_force_exit;
    ipc_event_loop_context_t* loop;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
};


/**
 * \brief Create the auth service instance.
 *
 * \param inst          The service instance to initialize.
 * \param auth          The auth socket to use for this instance.
 *
 * \returns a status code indicating success or failure.
 */
int auth_service_instance_init(auth_service_instance_t* inst, int auth);


/**
 * \brief Decode and dispatch requests received by the auth service.
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
 *      - AGENTD_ERROR_AUTHSERVICE_REQUEST_PACKET_INVALID_SIZE if the request
 *        packet size is invalid.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_AUTHSERVICE_IPC_WRITE_DATA_FAILURE if data could not be
 *        written to the client socket.
 */
int auth_service_decode_and_dispatch(
    auth_service_instance_t* inst, ipc_socket_context_t* sock, void* req,
    size_t size);


/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*AGENTD_AUTH_SERVICE_PRIVATE_HEADER_GUARD*/
