/**
 * \file agentd/ipc.h
 *
 * \brief Inter-process communication.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_IPC_HEADER_GUARD
#define AGENTD_IPC_HEADER_GUARD

#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

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

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_IPC_HEADER_GUARD*/
