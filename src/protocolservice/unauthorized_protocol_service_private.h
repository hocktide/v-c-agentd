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

#include <agentd/bitcap.h>
#include <agentd/dataservice.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/ipc.h>
#include <agentd/protocolservice/api.h>
#include <stdint.h>

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
    /** \brief Connection is closed. */
    UPCS_CLOSED,

    /** \brief Start by reading a handshake request from the client. */
    UPCS_READ_HANDSHAKE_REQ_FROM_CLIENT,

    /** \brief Gather entropy for the handshake process. */
    UPCS_HANDSHAKE_GATHER_ENTROPY,

    /** \brief Wait for entropy, but the connection has closed. */
    UPCS_HANDSHAKE_GATHER_ENTROPY_CLOSED,

    /** \brief Then write a handshake response to the client. */
    UPCS_WRITE_HANDSHAKE_RESP_TO_CLIENT,

    /** \brief Read a handshake acknowledge from the client. */
    UPCS_READ_HANDSHAKE_ACK_FROM_CLIENT,

    /** \brief Write the handshake acknowledge to the client. */
    UPCS_WRITE_HANDSHAKE_ACK_TO_CLIENT,

    /** \brief The client connection is closing due to an unauthorized state. */
    UPCS_UNAUTHORIZED,

    /** \brief Wait for data service child context. */
    APCS_DATASERVICE_CHILD_CONTEXT_WAIT,

    /** \brief Read a command from the client. */
    APCS_READ_COMMAND_REQ_FROM_CLIENT,

    /** \brief Write the command request to the application service. */
    APCS_WRITE_COMMAND_REQ_TO_APP,

    /** \brief Read the command response from the application service. */
    APCS_READ_COMMAND_RESP_FROM_APP,

    /** \brief Write the command response to the client. */
    APCS_WRITE_COMMAND_RESP_TO_CLIENT,

    /** \brief This connection is quiescing. */
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
    int dataservice_child_context;
    BITCAP(dataservice_caps, DATASERVICE_API_CAP_BITS_MAX);
    bool key_found;
    uint8_t entity_uuid[16];
    vccrypt_buffer_t entity_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_key_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret; /* TODO - move to auth service. */
    uint64_t client_iv;
    uint64_t server_iv;
    uint32_t current_request_offset;
    unauthorized_protocol_request_id_t request_id;
} unauthorized_protocol_connection_t;

/**
 * \brief Unauthorized protocol service instance.
 */
struct unauthorized_protocol_service_instance
{
    disposable_t hdr;
    unauthorized_protocol_connection_t* connections;
    size_t num_connections;
    unauthorized_protocol_connection_t* free_connection_head;
    unauthorized_protocol_connection_t* used_connection_head;
    unauthorized_protocol_connection_t* dataservice_context_create_head;
    /* TODO - hard-coded to current number of dataservice children. Should be
     * dynamically determined. */
    unauthorized_protocol_connection_t* dataservice_child_map[1024];
    ipc_socket_context_t random;
    ipc_socket_context_t data;
    ipc_socket_context_t proto;
    ipc_event_loop_context_t loop;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    vccrypt_buffer_t agent_pubkey;
    vccrypt_buffer_t agent_privkey;
    vccrypt_buffer_t authorized_entity_pubkey;
    uint8_t agent_id[16];
    uint8_t authorized_entity_id[16];
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
 * \brief Close a connection, returning it to the free connection pool.
 *
 * \param conn          The connection to close.
 */
void unauthorized_protocol_service_close_connection(
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
 * \param random        The random socket to use for this instance.
 * \param data          The dataservice socket to use for this instance.
 * \param proto         The protocol socket to use for this instance.
 * \param max_socks     The maximum number of socket connections to accept.
 *
 * \returns a status code indicating success or failure.
 */
int unauthorized_protocol_service_instance_init(
    unauthorized_protocol_service_instance_t* inst, int random, int data,
    int proto, size_t max_socks);

/**
 * \brief Handle read events on the protocol socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this proto socket.
 */
void unauthorized_protocol_service_ipc_read(
    ipc_socket_context_t* ctx, int event_flags,
    void* user_context);

/**
 * \brief Read data from a connection
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this proto socket.
 */
void unauthorized_protocol_service_connection_read(
    ipc_socket_context_t* ctx, int event_flags,
    void* user_context);

/**
 * \brief The write callback for managing writes to the client connection and
 * for advancing the state machine after the write is completed.
 *
 * \param ctx           The socket context for this write callback.
 * \param event_flags   The event flags that led to this callback being called.
 * \param user_context  The user context for this callback (expected: a protocol
 *                      connection).
 */
void unauthorized_protocol_service_connection_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Read data from the random service socket.
 *
 * \param ctx           The socket context triggering this event.
 * \param event_flags   The flags for this event.
 * \param user_context  The user context for this event.
 */
void unauthorized_protocol_service_random_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Write data to the random service socket.
 *
 * \param ctx           The socket context triggering this event.
 * \param event_flags   The flags for this event.
 * \param user_context  The user context for this event.
 */
void unauthorized_protocol_service_random_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Read data from the data service socket.
 *
 * \param ctx           The socket context for this read callback.
 * \param event_flags   The event flags that led to this callback being called.
 * \param user_context  The user context for this callback (expected: a protocol
 *                      service instance).
 */
void unauthorized_protocol_service_dataservice_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Write data to the dataservice socket.
 *
 * \param ctx           The socket context for this write callback.
 * \param event_flags   The event flags that led to this callback being called.
 * \param user_context  The user context for this callback (expected: a protocol
 *                      service instance).
 */
void unauthorized_protocol_service_dataservice_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Attempt to read a handshake request from the client.
 *
 * \param conn      The connection from which this handshake request should be
 *                  read.
 */
void unauthorized_protocol_service_connection_handshake_req_read(
    unauthorized_protocol_connection_t* conn);

/**
 * \brief Compute and write the handshake response for the handshake request.
 *
 * \param conn          The connection for which the response should be written.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero return code on failure.
 */
int unauthorized_protocol_service_write_handshake_request_response(
    unauthorized_protocol_connection_t* conn);

/**
 * \brief Attempt to the client challenge response acknowledgement.
 *
 * \param conn      The connection from which this ack should be read.
 */
void unauthorized_protocol_service_connection_handshake_ack_read(
    unauthorized_protocol_connection_t* conn);

/**
 * \brief Attempt to read a command from the client.
 *
 * \param conn      The connection from which this command should be read.
 */
void unauthorized_protocol_service_command_read(
    unauthorized_protocol_connection_t* conn);

/**
 * \brief Write a request to the random service to gather entropy.
 *
 * \param conn          The connection for which the request is written.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero return code on failure.
 */
int unauthorized_protocol_service_write_entropy_request(
    unauthorized_protocol_connection_t* conn);

/**
 * \brief Get the entity key associated with the data read during a handshake
 * request.
 *
 * \param conn          The connection for which the entity key should be
 *                      resolved.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero return code on failure.
 */
int unauthorized_protocol_service_get_entity_key(
    unauthorized_protocol_connection_t* conn);

/**
 * \brief Write an error response to the socket and set the connection state to
 * unauthorized.
 *
 * This method writes an error response to the socket and sets up the state
 * machine to close the connection after the error response is written.
 *
 * \param conn          The connection to which the error response is written.
 * \param request_id    The id of the request that caused the error.
 * \param status        The status code of the error.
 * \param offset        The request offset that caused the error.
 * \param encrypted     Set to true if this packet should be encrypted.
 */
void unauthorized_protocol_service_error_response(
    unauthorized_protocol_connection_t* conn, int request_id, int status,
    uint32_t offset, bool encrypted);

/**
 * \brief Decode and dispatch a request from the client.
 *
 * \param conn              The connection to close.
 * \param request_id        The request ID to decode.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_decode_and_dispatch(
    unauthorized_protocol_connection_t* conn, uint32_t request_id,
    uint32_t request_offset, const uint8_t* breq, size_t size);

/**
 * \brief Handle a latest_block_id_get request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_latest_block_id_get(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size);

/**
 * \brief Handle a transaction submit request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_transaction_submit(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size);

/**
 * \brief Handle a block by id get request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_block_by_id_get(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size);

/**
 * \brief Handle a get previous block id request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_block_id_get_prev(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size);

/**
 * \brief Handle a get next block id request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_block_id_get_next(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size);

/**
 * \brief Handle a transaction get by id request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_transaction_by_id_get(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size);

/**
 * \brief Handle a transaction get next id request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_txn_id_get_next(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size);

/**
 * \brief Handle a transaction get prev id request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_txn_id_get_prev(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size);

/**
 * \brief Handle a transaction get block id request.
 *
 * \param conn              The connection to close.
 * \param request_offset    The offset of the request.
 * \param breq              The bytestream of the request.
 * \param size              The size of this request bytestream.
 */
void unauthorized_protocol_service_handle_request_txn_id_get_block_id(
    unauthorized_protocol_connection_t* conn, uint32_t request_offset,
    const uint8_t* breq, size_t size);

/**
 * \brief Request that a dataservice child context be created.
 *
 * \param conn      The connection to be assigned a child context when this
 *                  request completes.
 *
 * This connection is pushed to the dataservice context create list, where it
 * will remain until the next dataservice context create request completes.
 */
void unauthorized_protocol_service_dataservice_request_child_context(
    unauthorized_protocol_connection_t* conn);

/**
 * Handle a child_context_create response.
 *
 * \param svc               The protocol service instance.
 * \param resp              The response from the child context create call.
 * \param resp_size         The size of the response.
 */
void ups_dispatch_dataservice_response_child_context_create(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size);

/**
 * Handle a child_context_close response.
 *
 * \param svc               The protocol service instance.
 * \param resp              The response from the child context create call.
 * \param resp_size         The size of the response.
 */
void ups_dispatch_dataservice_response_child_context_close(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size);

/**
 * Handle a block_id_latest_read response.
 *
 * \param svc               The protocol service instance.
 * \param resp              The response from the child context create call.
 * \param resp_size         The size of the response.
 */
void ups_dispatch_dataservice_response_block_id_latest_read(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size);

/**
 * Handle a transaction submit response.
 *
 * \param svc               The protocol service instance.
 * \param resp              The response from the child context create call.
 * \param resp_size         The size of the response.
 */
void ups_dispatch_dataservice_response_transaction_submit(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size);

/**
 * Handle a meta block read response.
 *
 * \param svc               The protocol service instance.
 * \param resp              The response from the child context create call.
 * \param resp_size         The size of the response.
 */
void ups_dispatch_dataservice_response_block_meta_read(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size);

/**
 * Handle a block read response.
 *
 * \param conn              The peer connection context.
 * \param dresp             The decoded response.
 */
void ups_dispatch_dataservice_response_block_read(
    unauthorized_protocol_connection_t* conn,
    const dataservice_response_block_get_t* dresp);

/**
 * Handle a block id read next response.
 *
 * \param conn              The peer connection context.
 * \param dresp             The decoded response.
 */
void ups_dispatch_dataservice_response_block_read_id_next(
    unauthorized_protocol_connection_t* conn,
    const dataservice_response_block_get_t* dresp);

/**
 * Handle a block id read prev response.
 *
 * \param conn              The peer connection context.
 * \param dresp             The decoded response.
 */
void ups_dispatch_dataservice_response_block_read_id_prev(
    unauthorized_protocol_connection_t* conn,
    const dataservice_response_block_get_t* dresp);

/**
 * Handle a meta transaction read response.
 *
 * \param svc               The protocol service instance.
 * \param resp              The response from the child context create call.
 * \param resp_size         The size of the response.
 */
void ups_dispatch_dataservice_response_transaction_meta_read(
    unauthorized_protocol_service_instance_t* svc, const void* resp,
    size_t resp_size);

/**
 * Handle a transaction read response.
 *
 * \param conn              The peer connection context.
 * \param dresp             The decoded response.
 */
void ups_dispatch_dataservice_response_transaction_read(
    unauthorized_protocol_connection_t* conn,
    const dataservice_response_canonized_transaction_get_t* dresp);

/**
 * Handle a transaction read next id response.
 *
 * \param conn              The peer connection context.
 * \param dresp             The decoded response.
 */
void ups_dispatch_dataservice_response_txn_read_id_next(
    unauthorized_protocol_connection_t* conn,
    const dataservice_response_canonized_transaction_get_t* dresp);

/**
 * Handle a transaction read prev id response.
 *
 * \param conn              The peer connection context.
 * \param dresp             The decoded response.
 */
void ups_dispatch_dataservice_response_txn_read_id_prev(
    unauthorized_protocol_connection_t* conn,
    const dataservice_response_canonized_transaction_get_t* dresp);

/**
 * Handle a transaction read block id response.
 *
 * \param conn              The peer connection context.
 * \param dresp             The decoded response.
 */
void ups_dispatch_dataservice_response_txn_read_block_id(
    unauthorized_protocol_connection_t* conn,
    const dataservice_response_canonized_transaction_get_t* dresp);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*AGENTD_UNAUTHORIZED_PROTOCOL_SERVICE_PRIVATE_HEADER_GUARD*/
