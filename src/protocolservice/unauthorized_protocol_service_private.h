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
    /** \brief Start by reading a handshake request from the client. */
    UPCS_READ_HANDSHAKE_REQ_FROM_CLIENT,

    /** \brief Then write a handshake response to the client. */
    UPCS_WRITE_HANDSHAKE_RESP_TO_CLIENT,

    /** \brief Read a handshake acknowledge from the client. */
    UPCS_READ_HANDSHAKE_ACK_FROM_CLIENT,

    /** \brief Write the handshake acknowledge to the client. */
    UPCS_WRITE_HANDSHAKE_ACK_TO_CLIENT,

    /** \brief The client connection is closing due to an unauthorized state. */
    UPCS_UNAUTHORIZED,

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
    bool key_found;
    uint8_t entity_uuid[16];
    vccrypt_buffer_t entity_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_key_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret; /* TODO - move to auth service. */
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
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    vccrypt_prng_context_t prng;
    vccrypt_mac_options_t short_hmac; /* XXX - eliminate with suite feature. */
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
