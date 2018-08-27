/**
 * \file agentd/ipc.h
 *
 * \brief Inter-process communication.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_IPC_HEADER_GUARD
#define AGENTD_IPC_HEADER_GUARD

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
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
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_socketpair(int domain, int type, int protocol, int* lhs, int* rhs);

/**
 * \brief Set a socket for synchronous (blocking) I/O.  Afterward, the
 * ipc_*_block socket I/O methods can be used.
 *
 * On success, sd is synchronous, and all I/O on this socket will block.
 *
 * \param sd            The socket descriptor to make synchronous.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_make_block(int sock);

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
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_make_noblock(
    int sock, ipc_socket_context_t* ctx, void* user_context);

/**
 * \brief Write a character string to the blocking socket.
 *
 * On success, the character string value is written, along with type
 * information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The string to write.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_write_string_block(int sock, const char* val);

/**
 * \brief Write a uint64_t value to the blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_write_uint64_block(int sock, uint64_t val);

/**
 * \brief Write an int64_t value to the blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_write_int64_block(int sock, int64_t val);

/**
 * \brief Write a uint8_t value to the blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_write_uint8_block(int sock, uint8_t val);

/**
 * \brief Write an int8_t value to the blocking socket.
 *
 * On success, the value is written, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The value to write.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_write_int8_block(int sock, int8_t val);

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
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_read_string_block(int sock, char** val);

/**
 * \brief Read a uint64_t value from the blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_read_uint64_block(int sock, uint64_t* val);

/**
 * \brief Read an int64_t value from the blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_read_int64_block(int sock, int64_t* val);

/**
 * \brief Read a uint8_t value from the blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_read_uint8_block(int sock, uint8_t* val);

/**
 * \brief Read an int8_t value from the blocking socket.
 *
 * On success, the value is read, along with type information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           Pointer to hold the value.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_read_int8_block(int sock, int8_t* val);

/**
 * \brief Initialize the event loop for handling IPC non-blocking I/O.
 *
 * On success, this event loop is owned by the caller and must be disposed when
 * no longer needed by calling the dispose() method.
 *
 * \param loop          The event loop context to initialize.
 *
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_event_loop_init(ipc_event_loop_context_t* loop);

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
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_event_loop_add(
    ipc_event_loop_context_t* loop, ipc_socket_context_t* sock);

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
 * \returns 0 on success and non-zero on failure.
 */
ssize_t ipc_event_loop_remove(
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

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_IPC_HEADER_GUARD*/
