/**
 * \file canonization/canonizationservice_internal.h
 *
 * \brief Internal header for the canonization service.
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_CANONIZATIONSERVICE_INTERNAL_HEADER_GUARD
#define AGENTD_CANONIZATIONSERVICE_INTERNAL_HEADER_GUARD

#include <agentd/dataservice/data.h>
#include <agentd/ipc.h>
#include <vccert/builder.h>
#include <vccrypt/suite.h>
#include <vpr/allocator.h>
#include <vpr/linked_list.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/* forward declaration for canonizationservice_transaction_t */
struct canonizationservice_transaction;
typedef struct canonizationservice_transaction
    canonizationservice_transaction_t;

/* forward declaration for canonizationservice_state_t */
enum canonizationservice_state;
typedef enum canonizationservice_state canonizationservice_state_t;

typedef struct canonizationservice_instance
{
    disposable_t hdr;
    bool configured;
    bool running;
    bool force_exit;
    int64_t block_max_milliseconds;
    size_t block_max_transactions;
    ipc_event_loop_context_t* loop_context;
    ipc_socket_context_t* data;
    ipc_socket_context_t* random;
    uint32_t data_child_context;
    ipc_timer_context_t timer;
    int state;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t crypto_suite;
    vccert_builder_options_t builder_opts;
    linked_list_options_t transaction_list_opts;
    uint8_t block_id[16];
    linked_list_t* transaction_list;
} canonizationservice_instance_t;

struct canonizationservice_transaction
{
    disposable_t hdr;
    data_transaction_node_t node;
    size_t cert_size;
    uint8_t cert[];
};

enum canonizationservice_state
{
    CANONIZATIONSERVICE_STATE_IDLE,
    CANONIZATIONSERVICE_STATE_WAITRESP_GET_RANDOM_BYTES,
    CANONIZATIONSERVICE_STATE_WAITRESP_CHILD_CONTEXT_CREATE,
    CANONIZATIONSERVICE_STATE_WAITRESP_LATEST_BLOCK_ID_GET,
    CANONIZATIONSERVICE_STATE_WAITRESP_BLOCK_GET,
    CANONIZATIONSERVICE_STATE_WAITRESP_PQ_TXN_FIRST_GET,
    CANONIZATIONSERVICE_STATE_WAITRESP_PQ_TXN_GET,
    CANONIZATIONSERVICE_STATE_WAITRESP_BLOCK_MAKE,
    CANONIZATIONSERVICE_STATE_WAITRESP_CHILD_CONTEXT_CLOSE
};

/**
 * \brief Create the canonization service instance.
 *
 * \returns a properly created canonization service instance, or NULL on
 * failure.
 */
canonizationservice_instance_t* canonizationservice_instance_create();

/**
 * \brief Dispose of a canonizationservice_transaction_t instance.
 *
 * \param ptr           Opaque pointer to the transaction instance to be
 *                      disposed.
 */
void canonizationservice_transaction_dispose(void* ptr);

/**
 * \brief List element disposer for the canonization service transaction linked
 * list.
 *
 * \param alloc_opts    Ignored by this disposer.
 * \param elem          The element to dispose.
 */
void canonizationservice_transaction_list_element_dispose(
    allocator_options_t* alloc_opts, void* elem);

/**
 * \brief Timer callback for the canonization service.
 *
 * This callback is called periodically to check the process queue for attested
 * certificates.  When these are found, these are used to build the next block
 * that is appended to the blockchain.
 *
 * \param timer         The timer context for this call.
 * \param context       The user context for this call, which is expected to be
 *                      a \ref canonizationservice_instance_t instance.
 */
void canonizationservice_timer_cb(ipc_timer_context_t* timer, void* context);

/**
 * \brief Decode and dispatch requests received by the canonization service on
 * the control socket.
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
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_REQUEST_PACKET_INVALID_SIZE if the
 *        request packet size is invalid.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered in this operation.
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_WRITE_DATA_FAILURE if data could
 *        not be written to the client socket.
 */
int canonizationservice_decode_and_dispatch_control_command(
    canonizationservice_instance_t* instance, ipc_socket_context_t* sock,
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
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_WRITE_DATA_FAILURE if data could
 *        not be written to the client socket.
 */
int canonizationservice_decode_and_dispatch_control_command_configure(
    canonizationservice_instance_t* instance, ipc_socket_context_t* sock,
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
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_WRITE_DATA_FAILURE if data could
 *        not be written to the client socket.
 */
int canonizationservice_decode_and_dispatch_control_command_start(
    canonizationservice_instance_t* instance, ipc_socket_context_t* sock,
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
 *      - AGENTD_ERROR_CANONIZATIONSERVICE_IPC_WRITE_DATA_FAILURE if data could
 *        not be written to the client socket
 */
int canonizationservice_decode_and_dispatch_write_status(
    ipc_socket_context_t* sock, uint32_t method, uint32_t offset,
    uint32_t status, void* data, size_t data_size);

/**
 * \brief Write a request to the random service to generate a block id.
 *
 * \param instance      The canonization service instance.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero return code on failure.
 */
int canonizationservice_write_block_id_request(
    canonizationservice_instance_t* instance);

/**
 * \brief Handle read events on the control socket.
 *
 * \param ctx               The non-blocking socket context.
 * \param event_flags       The event that triggered this callback.
 * \param user_context      The user context for this control socket.
 */
void canonizationservice_control_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Handle write events on the data socket.
 *
 * \param ctx           The non-blocking socket context.
 * \param event_flags   The event that triggered this callback.
 * \param user_context  The user context for this data socket.
 */
void canonizationservice_control_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Handle read events on the data socket.
 *
 * \param ctx               The non-blocking socket context.
 * \param event_flags       The event that triggered this callback.
 * \param user_context      The user context for this control socket.
 */
void canonizationservice_data_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Callback for writing data to the data service socket from the
 * canonization service.
 *
 * \param ctx           The socket context on which this write request occurred.
 * \param event_flags   The event flags that triggered this callback.
 * \param user_context  Opaque pointer to the canonization service instance.
 */
void canonizationservice_data_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Callback for writing data to the random service socket from the
 * canonization service.
 *
 * \param ctx           The socket context on which this write request occurred.
 * \param event_flags   The event flags that triggered this callback.
 * \param user_context  Opaque pointer to the canonization service instance.
 */
void canonizationservice_random_write(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Handle read events on the random socket.
 *
 * \param ctx               The non-blocking socket context.
 * \param event_flags       The event that triggered this callback.
 * \param user_context      The user context for this control socket.
 */
void canonizationservice_random_read(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Handle the response from the data service child context create call.
 *
 * \param instance      The canonization service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void canonizationservice_dataservice_response_child_context_create(
    canonizationservice_instance_t* instance, const uint32_t* resp,
    const size_t resp_size);

/**
 * \brief Handle the response from the data service child context close call.
 *
 * \param instance      The canonization service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void canonizationservice_dataservice_response_child_context_close(
    canonizationservice_instance_t* instance, const uint32_t* resp,
    const size_t resp_size);

/**
 * \brief Handle the response from the data service transaction first read.
 *
 * \param instance      The canonization service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void canonizationservice_dataservice_response_transaction_first_read(
    canonizationservice_instance_t* instance, const uint32_t* resp,
    const size_t resp_size);

/**
 * \brief Handle the response from the data service transaction read.
 *
 * \param instance      The canonization service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void canonizationservice_dataservice_response_transaction_read(
    canonizationservice_instance_t* instance, const uint32_t* resp,
    const size_t resp_size);

/**
 * \brief Handle the response from the data service block write.
 *
 * \param instance      The canonization service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void canonizationservice_dataservice_response_block_write(
    canonizationservice_instance_t* instance, const uint32_t* resp,
    const size_t resp_size);

/**
 * \brief Send a child context create request to the data service.
 *
 * \param instance      The canonization service instance.
 */
int canonizationservice_dataservice_sendreq_child_context_create(
    canonizationservice_instance_t* instance);

/**
 * \brief Build a new block for the blockchain, using the currently attested
 * transactions.
 *
 * \param instance      The canonization service instance.
 */
int canonizationservice_block_make(
    canonizationservice_instance_t* instance);

/**
 * \brief Close the child context, leading to reset of the canonization service.
 *
 * \param instance      The canonization service instance.
 */
void canonizationservice_child_context_close(
    canonizationservice_instance_t* instance);

/**
 * \brief Clean up and reset the canonization service.
 *
 * \param instance      The canonization service instance.
 * \param should_sleep  If set, wake up on the sleep timer.  If not set, call
 *                      the sleep timer callback right away.
 */
void canonizationservice_reset(
    canonizationservice_instance_t* instance, bool should_sleep);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_CANONIZATIONSERVICE_INTERNAL_HEADER_GUARD*/
