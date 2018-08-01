/**
 * \file agentd/ipc.h
 *
 * \brief Inter-process communication.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_IPC_HEADER_GUARD
#define AGENTD_IPC_HEADER_GUARD

#include <stdint.h>
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

/**
 * \brief Socket context used for asynchronous (non-blocking) I/O.  Contains an
 * opaque reference to the underlying async I/O implementation.
 *
 * Is disposable.
 */
typedef struct ipc_socket_context
{
    disposable_t hdr;
    void* impl;
} ipc_socket_context_t;

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
int ipc_socketpair(int domain, int type, int protocol, int* lhs, int* rhs);

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
 * \param sd            The socket descriptor to make asynchronous.
 * \param ctx           The socket context to initialize using this call.
 *
 * \returns 0 on success and non-zero on failure.
 */
int ipc_make_noblock(int sock, ipc_socket_context_t* ctx);

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
int ipc_write_string_block(int sock, const char* val);

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
int ipc_write_uint64_block(int sock, uint64_t val);

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
int ipc_write_int64_block(int sock, int64_t val);

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
int ipc_write_uint8_block(int sock, uint8_t val);

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
int ipc_write_int8_block(int sock, int8_t val);

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
int ipc_read_string_block(int sock, char** val);

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
int ipc_read_uint64_block(int sock, uint64_t* val);

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
int ipc_read_int64_block(int sock, int64_t* val);

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
int ipc_read_uint8_block(int sock, uint8_t* val);

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
int ipc_read_int8_block(int sock, int8_t* val);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_IPC_HEADER_GUARD*/
