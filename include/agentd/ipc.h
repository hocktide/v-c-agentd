/**
 * \file agentd/ipc.h
 *
 * \brief Inter-process communication.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_IPC_HEADER_GUARD
#define AGENTD_IPC_HEADER_GUARD

#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vccrypt/suite.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

#define IPC_DATA_TYPE_BOM 0x00
#define IPC_DATA_TYPE_UINT8 0x01
#define IPC_DATA_TYPE_UINT32 0x03
#define IPC_DATA_TYPE_UINT64 0x04
#define IPC_DATA_TYPE_INT8 0x09
#define IPC_DATA_TYPE_INT32 0x0A
#define IPC_DATA_TYPE_INT64 0x0B
#define IPC_DATA_TYPE_STRING 0x10
#define IPC_DATA_TYPE_DATA_PACKET 0x20
#define IPC_DATA_TYPE_AUTHED_PACKET 0x30
#define IPC_DATA_TYPE_EOM 0xFF

/* forward decl for ipc_socket_context. */
struct ipc_socket_context;

/**
 * \brief Socket event flags.
 */
typedef enum ipc_socket_event_flags_enum
{
    /**
     * \brief The socket is available for reading.
     */
    IPC_SOCKET_EVENT_READ = 0x01,

    /**
     * \brief The socket is available for writing.
     */
    IPC_SOCKET_EVENT_WRITE = 0x02,

    /**
     * \brief An error has occurred on this socket.
     */
    IPC_SOCKET_EVENT_ERROR = 0x04

} ipc_socket_event_flags_enum_t;

/**
 * \brief Callback method for an IPC socket event.
 *
 * \param ctx           The non-blocking socket context in which this event
 *                      occurred.
 * \param event_flags   The event flags that caused this callback.  See
 *                      \ref ipc_socket_event_flags_enum_t.
 * \param user_context  The user context associated with this socket.
 */
typedef void (*ipc_socket_event_cb_t)(
    struct ipc_socket_context* ctx, int event_flags, void* user_context);

/**
 * \brief Socket context used for asynchronous (non-blocking) I/O.  Contains an
 * opaque reference to the underlying async I/O implementation.
 *
 * Is disposable.
 */
typedef struct ipc_socket_context
{
    disposable_t hdr;
    int fd;
    ipc_socket_event_cb_t read;
    ipc_socket_event_cb_t write;
    void* impl;
    void* user_context;
} ipc_socket_context_t;

/**
 * \brief IPC Event Loop context.
 *
 * Is disposable.
 */
typedef struct ipc_event_loop_context
{
    disposable_t hdr;
    void* impl;
} ipc_event_loop_context_t;

/**
 * \brief Create a socket pair of the given type and protocol for the given
 * domain.
 *
 * On success, lhs and rhs are set to the left-hand and right-hand sides of the
 * socket pair.
 *
 * \param domain        The domain for the socket.
 * \param type          The type of socket.
 * \param protocol      The protocol for the socket. 
 * \param lhs           Pointer to the integer variable updated to the
 *                      left-hand-side descriptor.
 * \param rhs           Pointer to the integer variable updated to the
 *                      right-hand-side descriptor for the socket pair.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_SOCKETPAIR_FAILURE if creating the socket pair
 *        failed.
 */
int ipc_socketpair(int domain, int type, int protocol, int* lhs, int* rhs);

/**
 * \brief Set a socket for synchronous (blocking) I/O.  Afterward, the
 * ipc_*_block socket I/O methods can be used.
 *
 * On success, sd is synchronous, and all I/O on this socket will block.
 *
 * \param sd            The socket descriptor to make synchronous.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_FCNTL_GETFL_FAILURE if the fcntl flags could not be
 *        read.
 *      - AGENTD_ERROR_IPC_FCNTL_SETFL_FAILURE if the fcntl flags could not be
 *        updated.
 */
int ipc_make_block(int sock);

/**
 * \brief Set a socket for asynchronous (non-blocking) I/O.  Afterward, the
 * ipc_*_noblock socket I/O methods can be used.
 *
 * On success, sd is asynchronous, and all I/O on this socket will not block.
 * As such, all I/O should be done using \ref ipc_socket_context_t.
 * Furthermore, the ipc_socket_context_t structure is owned by the caller and
 * must be disposed using the dispose() method.
 *
 * \param sock          The socket descriptor to make asynchronous.
 * \param ctx           The socket context to initialize using this call.
 * \param user_context  The user context for this connection.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition
 *        occurred during this operation.
 *      - AGENTD_ERROR_IPC_FCNTL_GETFL_FAILURE if the fcntl flags could not be
 *        read.
 *      - AGENTD_ERROR_IPC_FCNTL_SETFL_FAILURE if the fcntl flags could not be
 *        updated.
 */
int ipc_make_noblock(
    int sock, ipc_socket_context_t* ctx, void* user_context);

/**
 * \brief Write a raw data packet.
 *
 * On success, the raw data packet value will be written, along with type
 * information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The raw data to write.
 * \param size          The size of the raw data to write.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if writing data failed.
 */
int ipc_write_data_block(int sock, const void* val, uint32_t size);

/**
 * \brief Write an authenticated data packet.
 *
 * On success, the authenticated data packet value will be written, along with
 * type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param iv            The 64-bit IV to use for this packet.
 * \param val           The payload data to write.
 * \param size          The size of the payload data to write.
 * \param suite         The crypto suite to use for authenticating this packet.
 * \param secret        The shared secret between the peer and host.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if writing data failed.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_CRYPTO_SUITE if the crypto suite is
 *        invalid.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_SECRET if the secret key is invalid.
 */
int ipc_write_authed_data_block(
    int sock, uint64_t iv, const void* val, uint32_t size,
    vccrypt_suite_options_t* suite, vccrypt_buffer_t* secret);

/**
 * \brief Write a character string to the blocking socket.
 *
 * On success, the character string value is written, along with type
 * information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The string to write.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if writing data failed.
 */
int ipc_write_string_block(int sock, const char* val);

/**
 * \brief Write a uint64_t value to the blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if writing data failed.
 */
int ipc_write_uint64_block(int sock, uint64_t val);

/**
 * \brief Write an int64_t value to the blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if writing data failed.
 */
int ipc_write_int64_block(int sock, int64_t val);

/**
 * \brief Write a uint8_t value to the blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if writing data failed.
 */
int ipc_write_uint8_block(int sock, uint8_t val);

/**
 * \brief Write an int8_t value to the blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if writing data failed.
 */
int ipc_write_int8_block(int sock, int8_t val);

/**
 * \brief Read a raw data packet from the blocking socket.
 *
 * On success, a raw data buffer is allocated and read, along with type
 * information and size.  The caller owns this buffer and is responsible for
 * freeing it when it is no longer in use.
 *
 * \param sd            The socket descriptor from which the value is read.
 * \param val           Pointer to the pointer of the raw data buffer.
 * \param size          Pointer to the variable to receive the size of this
 *                      packet.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int ipc_read_data_block(int sock, void** val, uint32_t* size);

/**
 * \brief Read an authenticated data packet from the blocking socket.
 *
 * On success, an authenticated data buffer is allocated and read, along with
 * type information and size.  The caller owns this buffer and is responsible
 * for freeing it when it is no longer in use.
 *
 * \param sd            The socket descriptor from which the value is read.
 * \param iv            The 64-bit IV to expect for this packet.
 * \param val           Pointer to the pointer of the raw data buffer.
 * \param size          Pointer to the variable to receive the size of this
 *                      packet.
 * \param suite         The crypto suite to use for authenticating this packet.
 * \param secret        The shared secret between the peer and host.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_CRYPTO_SUITE if the crypto suite is
 *        invalid.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_SECRET if the secret key is invalid.
 *      - AGENTD_ERROR_IPC_AUTHENTICATION_FAILURE if the packet could not be
 *        authenticated.
 */
int ipc_read_authed_data_block(
    int sock, uint64_t iv, void** val, uint32_t* size,
    vccrypt_suite_options_t* suite, vccrypt_buffer_t* secret);

/**
 * \brief Read a character string from the blocking socket.
 *
 * On success, a character string value is allocated and read, along with type
 * information and size.  The caller owns this character string and is
 * responsible for freeing it when it is no longer in use.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to the string pointer to hold the string value
 *                      on success.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error.
 */
int ipc_read_string_block(int sock, char** val);

/**
 * \brief Read a uint64_t value from the blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if the data size read from
 *        the socket was unexpected.
 */
int ipc_read_uint64_block(int sock, uint64_t* val);

/**
 * \brief Read an int64_t value from the blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if the data size read from
 *        the socket was unexpected.
 */
int ipc_read_int64_block(int sock, int64_t* val);

/**
 * \brief Read a uint8_t value from the blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if the data size read from
 *        the socket was unexpected.
 */
int ipc_read_uint8_block(int sock, uint8_t* val);

/**
 * \brief Read an int8_t value from the blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if a blocking read on the socket
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read from
 *        the socket was unexpected.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if the data size read from
 *        the socket was unexpected.
 */
int ipc_read_int8_block(int sock, int8_t* val);

/**
 * \brief Initialize the event loop for handling IPC non-blocking I/O.
 *
 * On success, this event loop is owned by the caller and must be disposed when
 * no longer needed by calling the dispose() method.
 *
 * \param loop          The event loop context to initialize.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory.
 *      - AGENTD_ERROR_IPC_EVENT_BASE_NEW_FAILURE if event_base_new() failed.
 */
int ipc_event_loop_init(ipc_event_loop_context_t* loop);

/**
 * \brief Add a non-blocking socket to the event loop.
 *
 * On success, the event loop will manage events on this non-blocking socket.
 * Note that the ownership for this socket context remains with the caller.  It
 * is the caller's responsibility to remove this socket from the event loop and
 * to dispose the socket.
 *
 * \param loop          The event loop context to which this socket is added.
 * \param sock          The socket context to add to the event loop.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_INVALID_ARGUMENT if the socket context has already
 *        been added to an event loop.
 *      - AGENTD_ERROR_IPC_MISSING_CALLBACK if either a read or write callback
 *        has not been set.
 *      - AGENTD_ERROR_IPC_EVBUFFER_NEW_FAILURE if a new event buffer could not
 *        be created.
 *      - AGENTD_ERROR_IPC_EVENT_NEW_FAILURE if a new event could not be
 *        created.
 *      - AGENTD_ERROR_IPC_EVENT_ADD_FAILURE if the event cannot be added to the
 *        event loop.
 */
int ipc_event_loop_add(
    ipc_event_loop_context_t* loop, ipc_socket_context_t* sock);

/**
 * \brief Exit the event loop when the given signal is caught.
 *
 * On success, the event loop will exit when this signal is caught by the signal
 * handler.
 *
 * \param loop          The event loop context to exit when the signal is
 *                      caught.
 * \param sig           The signal that triggers this exit.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered.
 *      - AGENTD_ERROR_IPC_EVSIGNAL_NEW_FAILURE if a new signal event could not
 *        be created.
 *      - AGENTD_ERROR_IPC_EVENT_ADD_FAILURE if the signal event could not be
 *        added to the event base.
 */
int ipc_exit_loop_on_signal(
    ipc_event_loop_context_t* loop, int sig);

/**
 * \brief Instruct the loop to exit as soon as all events are processed.
 *
 * \param loop          The event loop context to exit.
 */
void ipc_exit_loop(ipc_event_loop_context_t* loop);

/**
 * \brief Remove a non-blocking socket from the event loop.
 *
 * On success, the event loop will no longer manage events on this non-blocking
 * socket.  Note that the ownership for this socket context remains with the
 * caller.  It is the caller's responsibility to dispose the socket.
 *
 * \param loop          The event loop context from which this socket is
 *                      removed.
 * \param sock          This socket context is removed from the event loop.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_INVALID_ARGUMENT if the socket context does not
 *        belong to a loop.
 */
int ipc_event_loop_remove(
    ipc_event_loop_context_t* loop, ipc_socket_context_t* sock);

/**
 * \brief Run the event loop for IPC non-blocking I/O.
 *
 * \param loop          The event loop context to run.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_event_loop_run(ipc_event_loop_context_t* loop);

/**
 * \brief Get the number of bytes available in the write buffer.
 *
 * \note This method can only be called after a socket has been added to the
 * event loop.  Otherwise, the result is undefined.
 *
 * \param sock          The non-blocking socket to query.
 *
 * \returns a number greater than or equal to zero, indicating the number of
 * bytes available for writing.
 */
size_t ipc_socket_writebuffer_size(ipc_socket_context_t* sock);

/**
 * \brief Write data from the write buffer to the non-blocking socket, returning
 * either the number of bytes written, or a value indicating error.
 *
 * \note This method can only be called after a socket has been added to the
 * event loop.  Otherwise, the result is undefined.
 *
 * \param sock          The non-blocking socket for the write.
 *
 * \returns the number of bytes written, -1 on error, or 0.  If 0 is returned
 * AND the socket is available for writing via a write callback, then this
 * indicates that the socket has been closed by the peer.  If -1 is returned,
 * then errno should be checked to see if this is a real error or if the write
 * failed because it would block (EAGAIN / EWOULDBLOCK).
 */
ssize_t ipc_socket_write_from_buffer(ipc_socket_context_t* sock);

/**
 * \brief Get the number of bytes available in the read buffer.
 *
 * \note This method can only be called after a socket has been added to the
 * event loop.  Otherwise, the result is undefined.
 *
 * \param sock          The non-blocking socket to query.
 *
 * \returns a number greater than or equal to zero, indicating the number of
 * bytes in the read buffer.
 */
size_t ipc_socket_readbuffer_size(ipc_socket_context_t* sock);

/**
 * \brief Read data from the socket and place this into the readbuffer.
 *
 * \note This method can only be called after a socket has been added to the
 * event loop.  Otherwise, the result is undefined.
 *
 * \param sock          The non-blocking socket for the read.
 *
 * \returns the number of bytes read, -1 on error, or 0.  If 0 is returned
 * AND the socket is available for reading via a read callback, then this
 * indicates that the socket has been closed by the peer.  If -1 is returned,
 * then errno should be checked to see if this is a real error or if the read
 * failed because it would block (EAGAIN / EWOULDBLOCK).
 */
ssize_t ipc_socket_read_to_buffer(ipc_socket_context_t* sock);

/**
 * \brief Set the read event callback for a non-blocking socket.
 *
 * \note This method can only be called BEFORE a socket has been added to the
 * event loop.  Otherwise, the callback will not be properly set.
 *
 * \param sock          The socket to set.
 * \param cb            The callback to set.
 */
void ipc_set_readcb_noblock(
    ipc_socket_context_t* sock, ipc_socket_event_cb_t cb);

/**
 * \brief Set the write event callback for a non-blocking socket.
 *
 * \note This method can only be called BEFORE a socket has been added to the
 * event loop.  Otherwise, the callback will not be properly set.
 *
 * \param sock          The socket to set.
 * \param cb            The callback to set.
 */
void ipc_set_writecb_noblock(
    ipc_socket_context_t* sock, ipc_socket_event_cb_t cb);

/**
 * \brief Accept a connection from a listen socket.
 *
 * On success, the socket specified by sock contains a connection to a remote
 * peer.  The address parameter contains data about the peer.
 *
 * \param ctx           The non-blocking socket context from which a connection
 *                      is accepted.
 * \param sock          A pointer to a socket descriptor that is populated with
 *                      the socket connection to the remote peer on success.
 * \param addr          A pointer to the buffer to hold the peer address.  This
 *                      is populated with the peer address on success.
 * \param addrsize      The value pointed to by addrsize should be set to the
 *                      maximum value of this buffer.  It is set to the number
 *                      of bytes used by the address on success.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if this operation would cause the socket
 *        to block.
 *      - AGENTD_ERROR_IPC_ACCEPT_NOBLOCK_FAILURE if accepting this socket
 *        failed.
 */
int ipc_accept_noblock(
    ipc_socket_context_t* ctx, int* sock, struct sockaddr* addr,
    socklen_t* addrsize);

/**
 * \brief Send a socket descriptor to the unix domain peer.
 *
 * On success, the socket sendsock is sent over the unix domain socket sock.
 * The caller maintains the local socket handle, and this should be closed by
 * the caller.
 *
 * \param sock          The unix domain socket through which sendsock should be
 *                      sent.
 * \param sendsock      The socket to send to the peer.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE if this operation failed.
 */
int ipc_sendsocket_block(int sock, int sendsock);

/**
 * \brief Receive a socket descriptor from the unix domain peer.
 *
 * On success, the socket recvsock is received from the unix domain socket.
 * The caller owns the local socket handle recvsock and must close it when no
 * longer needed.
 *
 * \param ctx           The unix domain socket from which recvsock should be
 *                      received.
 * \param recvsock      The socket to receive from the peer.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if this operation would block.
 *      - AGENTD_ERROR_IPC_READ_BLOCK_FAILURE if this operation failed.
 */
int ipc_receivesocket_noblock(ipc_socket_context_t* ctx, int* recvsock);

/**
 * \brief Write a raw data packet to a non-blocking socket.
 *
 * On success, the raw data packet value will be written, along with type
 * information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The raw data to write.
 * \param size          The size of the raw data to write.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE if adding the type
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE if adding the size
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE if adding the
 *        payload data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_NONBLOCK_FAILURE if a non-blocking write
 *        failed.
 */
int ipc_write_data_noblock(
    ipc_socket_context_t* sock, const void* val, uint32_t size);

/**
 * \brief Write an authenticated data packet to a non-blocking socket.
 *
 * On success, the authenticated data packet value will be written, along with
 * type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param iv            The 64-bit IV to use for this packet.
 * \param val           The raw data to write.
 * \param size          The size of the raw data to write.
 * \param suite         The crypto suite to use for authenticating this packet.
 * \param secret        The shared secret between the peer and host.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE if adding the type
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE if adding the size
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE if adding the
 *        payload data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_NONBLOCK_FAILURE if a non-blocking write
 *        failed.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_CRYPTO_SUITE if the crypto suite is
 *        invalid.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_SECRET if the secret key is invalid.
 */
int ipc_write_authed_data_noblock(
    ipc_socket_context_t* sock, uint64_t iv, const void* val, uint32_t size,
    vccrypt_suite_options_t* suite, vccrypt_buffer_t* secret);

/**
 * \brief Write a character string to a non-blocking socket.
 *
 * On success, the character string value is written, along with type
 * information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The string to write.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE if adding the type
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE if adding the size
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE if adding the
 *        payload data to the write buffer failed.
 */
int ipc_write_string_noblock(ipc_socket_context_t* sock, const char* val);

/**
 * \brief Write a uint64_t value to a non-blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE if adding the type
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE if adding the size
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE if adding the
 *        payload data to the write buffer failed.
 */
int ipc_write_uint64_noblock(ipc_socket_context_t* sock, uint64_t val);

/**
 * \brief Write an int64_t value to a non-blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE if adding the type
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE if adding the size
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE if adding the
 *        payload data to the write buffer failed.
 */
int ipc_write_int64_noblock(ipc_socket_context_t* sock, int64_t val);

/**
 * \brief Write a uint8_t value to a non-blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE if adding the type
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE if adding the size
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE if adding the
 *        payload data to the write buffer failed.
 */
int ipc_write_uint8_noblock(ipc_socket_context_t* sock, uint8_t val);

/**
 * \brief Write an int8_t value to a non-blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE if adding the type
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE if adding the size
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE if adding the
 *        payload data to the write buffer failed.
 */
int ipc_write_int8_noblock(ipc_socket_context_t* sock, int8_t val);

/**
 * \brief Read a raw data packet from a non-blocking socket.
 *
 * On success, a raw data buffer is allocated and read, along with type
 * information and size.  The caller owns this buffer and is responsible for
 * freeing it when it is no longer in use.
 *
 * \param sd            The socket descriptor from which the value is read.
 * \param val           Pointer to the pointer of the raw data buffer.
 * \param size          Pointer to the variable to receive the size of this
 *                      packet.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if the the operation was halted because
 *        it would block this thread.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if an unexpected data type
 *        was encountered when attempting to read this value.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if an unexpected data size
 *        was encountered when attempting to read this value.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error condition while executing.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE if draining the read buffer
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE if removing the payload
 *        from the read buffer failed.
 */
int ipc_read_data_noblock(
    ipc_socket_context_t* sock, void** val, uint32_t* size);

/**
 * \brief Read an authenticated data packet from a non-blocking socket.
 *
 * On success, an authenticated data buffer is allocated and read, along with
 * type information and size.  The caller owns this buffer and is responsible
 * for freeing it when it is no longer in use.
 *
 * \param sd            The socket descriptor from which the value is read.
 * \param iv            The 64-bit IV to expect for this packet.
 * \param val           Pointer to the pointer of the raw data buffer.
 * \param size          Pointer to the variable to receive the size of this
 *                      packet.
 * \param suite         The crypto suite to use for authenticating this packet.
 * \param secret        The shared secret between the peer and host.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if the the operation was halted because
 *        it would block this thread.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if an unexpected data type
 *        was encountered when attempting to read this value.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if an unexpected data size
 *        was encountered when attempting to read this value.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error condition while executing.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE if draining the read buffer
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE if removing the payload
 *        from the read buffer failed.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_CRYPTO_SUITE if the crypto suite is
 *        invalid.
 *      - AGENTD_ERROR_IPC_AUTHED_INVALID_SECRET if the secret key is invalid.
 *      - AGENTD_ERROR_IPC_AUTHENTICATION_FAILURE if the packet could not be
 *        authenticated.
 */
int ipc_read_authed_data_noblock(
    ipc_socket_context_t* sock, uint64_t iv, void** val, uint32_t* size,
    vccrypt_suite_options_t* suite, vccrypt_buffer_t* secret);

/**
 * \brief Read a character string from a non-blocking socket.
 *
 * On success, a character string value is allocated and read, along with type
 * information and size.  The caller owns this character string and is
 * responsible for freeing it when it is no longer in use.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to the string pointer to hold the string value
 *                      on success.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if the the operation was halted because
 *        it would block this thread.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if an unexpected data type
 *        was encountered when attempting to read this value.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if an unexpected data size
 *        was encountered when attempting to read this value.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if this operation encountered an
 *        out-of-memory error condition while executing.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE if draining the read buffer
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE if removing the payload
 *        from the read buffer failed.
 */
int ipc_read_string_noblock(ipc_socket_context_t* sock, char** val);

/**
 * \brief Read a uint64_t value from a non-blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if the the operation was halted because
 *        it would block this thread.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read was
 *        unexpected.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if the data size read was
 *        unexpected.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE if draining the read buffer
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE if removing the payload
 *        from the read buffer failed.
 */
int ipc_read_uint64_noblock(ipc_socket_context_t* sock, uint64_t* val);

/**
 * \brief Read an int64_t value from a non-blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if the the operation was halted because
 *        it would block this thread.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read was
 *        unexpected.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if the data size read was
 *        unexpected.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE if draining the read buffer
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE if removing the payload
 *        from the read buffer failed.
 */
int ipc_read_int64_noblock(ipc_socket_context_t* sock, int64_t* val);

/**
 * \brief Read a uint8_t value from a non-blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if the the operation was halted because
 *        it would block this thread.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read was
 *        unexpected.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if the data size read was
 *        unexpected.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE if draining the read buffer
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE if removing the payload
 *        from the read buffer failed.
 */
int ipc_read_uint8_noblock(ipc_socket_context_t* sock, uint8_t* val);

/**
 * \brief Read an int8_t value from a non-blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WOULD_BLOCK if the the operation was halted because
 *        it would block this thread.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE if the data type read was
 *        unexpected.
 *      - AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_SIZE if the data size read was
 *        unexpected.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_DRAIN_FAILURE if draining the read buffer
 *        failed.
 *      - AGENTD_ERROR_IPC_READ_BUFFER_REMOVE_FAILURE if removing the payload
 *        from the read buffer failed.
 */
int ipc_read_int8_noblock(ipc_socket_context_t* sock, int8_t* val);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_IPC_HEADER_GUARD*/
