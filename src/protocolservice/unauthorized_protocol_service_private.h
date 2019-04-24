/**
 * \file protocolservice/unauthorized_protocol_service_private.h
 *
 * \brief Private unauthorized protocol service functions and data structures.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_UNAUTHORIZED_PROTOCOL_SERVICE_PRIVATE_HEADER_GUARD
#define AGENTD_UNAUTHORIZED_PROTOCOL_SERVICE_PRIVATE_HEADER_GUARD

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/* Forward decl for unauthorized protocol service data structure. */
struct unauthorized_protocol_service_instance;

/* Forward decl for an unauthorized protocol connection. */
struct unauthorized_protocol_connection;

/**
 * \brief The unauthorized protocol service instance.
 */
typedef struct unauthorized_protocol_service_instance
    unauthorized_protocol_service_instance_t;

/**
 * \brief An unauthorized protocol connection.
 */
typedef struct unauthorized_protocol_connection
    unauthorized_protocol_connection_t;

/**
 * \brief States for an unauthorized protocol socket.
 */
typedef enum unauthorized_protocol_connection_state
{
    UPCS_READ_HANDSHAKE_REQ_FROM_CLIENT,
    UPCS_WRITE_HANDSHAKE_RESP_TO_CLIENT,
    UPCS_READ_HANDSHAKE_ACK_FROM_CLIENT,
    UPCS_WRITE_HANDSHAKE_ACK_TO_CLIENT,
    UPCS_UNAUTHORIZED,
    APCS_READ_COMMAND_REQ_FROM_CLIENT,
    APCS_WRITE_COMMAND_REQ_TO_APP,
    APCS_READ_COMMAND_RESP_FROM_APP,
    APCS_WRITE_COMMAND_RESP_TO_CLIENT,
    APCS_QUIESCING,
} unauthorized_protocol_connection_state_t;

/**
 * \brief Context for an unauthorized protocol connection.
 */
typedef struct unauthorized_protocol_connection
{
    disposable_t hdr;
    unauthorized_protocol_connection_t* prev;
    unauthorized_protocol_connection_t* next;
    ipc_socket_context_t ctx;
    unauthorized_protocol_connection_state_t state;
    unauthorized_protocol_service_instance_t* svc;
} unauthorized_protocol_connection_t;

/**
 * \brief Unauthorized protocol service instance.
 */
struct unauthorized_protocol_service_instance
{
    disposable_t hdr;
    unauthorized_protocol_connection_t* free_connection_head;
    unauthorized_protocol_connection_t* used_connection_head;
    ipc_socket_context_t proto;
    ipc_event_loop_context_t loop;
};

/**
 * \brief Initialize an unauthorized protocol connection instance.
 *
 * This instance takes ownership of the socket, which will be closed when this
 * instance is disposed.  This is different than the default behavior of
 * ipc_make_noblock.
 *
 * \param conn          The connection to initialize.
 * \param sock          The socket descriptor for this instance.
 * \param svc           The unauthorized protocol service instance.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE if a failure
 *        occurred when attempting to make a non-blocking socket.
 */
int unauthorized_protocol_connection_init(
    unauthorized_protocol_connection_t* conn, int sock,
    unauthorized_protocol_service_instance_t* svc);

/**
 * \brief Remove a protocol connection from its current list.
 *
 * \param head          Pointer to the head of the list.
 * \param conn          The connection to remove.
 */
void unauthorized_protocol_connection_remove(
    unauthorized_protocol_connection_t** head,
    unauthorized_protocol_connection_t* conn);

/**
 * \brief Push a protocol connection onto the given list.
 *
 * \param head          Pointer to the head of the list.
 * \param conn          The connection to push.
 */
void unauthorized_protocol_connection_push_front(
    unauthorized_protocol_connection_t** head,
    unauthorized_protocol_connection_t* conn);

/**
 * \brief Create the unauthorized protocol service instance.
 *
 * \param inst          The service instance to initialize.
 * \param proto         The protocol socket to use for this instance.
 * \param max_socks     The maximum number of socket connections to accept.
 *
 * \returns a status code indicating success or failure.
 */
int unauthorized_protocol_service_instance_init(
    unauthorized_protocol_service_instance_t* inst, int proto,
    size_t max_socks);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*AGENTD_UNAUTHORIZED_PROTOCOL_SERVICE_PRIVATE_HEADER_GUARD*/
