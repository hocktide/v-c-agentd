/**
 * \file ipc/ipc_internal.h
 *
 * \brief Inter-process communication internal details.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_IPC_IPC_INTERNAL_HEADER_GUARD
#define AGENTD_IPC_IPC_INTERNAL_HEADER_GUARD

#include <agentd/ipc.h>
#include <event.h>
#include <stdint.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Internal context for non-blocking sockets.
 */
typedef struct ipc_socket_impl
{
    struct event* read_ev;
    struct event* write_ev;
    struct evbuffer* readbuf;
    struct evbuffer* writebuf;
} ipc_socket_impl_t;

/**
 * \brief Internal context for signal handling events.
 */
typedef struct ipc_signal_event_impl
{
    struct ipc_signal_event_impl* next;
    struct event* ev;
} ipc_signal_event_impl_t;

/**
 * \brief Internal context for event loops.
 */
typedef struct ipc_event_loop_impl
{
    struct event_base* evb;
    ipc_signal_event_impl_t* sig_head;
} ipc_event_loop_impl_t;

/**
 * \brief Event loop callback.  Decode an event and send it to the ipc callback.
 *
 * \param fd        The socket file descriptor for this callback.
 * \param what      The flags for this event.
 * \param ctx       The user context for this event.
 */
void ipc_event_loop_cb(
    evutil_socket_t UNUSED(fd), short what, void* ctx);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_IPC_IPC_INTERNAL_HEADER_GUARD*/
