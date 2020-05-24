/**
 * \file listenservice/listenservice_internal.h
 *
 * \brief Internal header for the listen service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_LISTENSERVICE_INTERNAL_HEADER_GUARD
#define AGENTD_LISTENSERVICE_INTERNAL_HEADER_GUARD

#include <agentd/listenservice.h>
#include <agentd/ipc.h>
#include <event.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Instance type for listen service.
 */
typedef struct listenservice_instance
{
    ipc_event_loop_context_t* loop_context;
    bool listenservice_force_exit;
    int acceptsock;
} listenservice_instance_t;

/**
 * \brief Count the number of listen sockets, returning this number as an
 * integer.
 *
 * \param listenstart       The starting socket from which the count starts.
 *
 * \returns the number of valid descriptors found.
 */
int listenservice_count_sockets(int listenstart);

/**
 * \brief Read callback on listen sockets to accept a new socket.
 *
 * This callback is registered as part of the ipc callback mechanism for a
 * listen socket.  It forwards a socket to the accept socket in the \ref
 * listenservice_instance_t context structure.
 */
void listenservice_ipc_accept(
    ipc_socket_context_t* ctx, int event_flags, void* user_context);

/**
 * \brief Set up a clean re-entry from the event loop and ensure that no other
 * callbacks occur by setting the appropriate force exit flag.
 *
 * \param instance      The listenservice instance.
 */
void listenservice_exit_event_loop(listenservice_instance_t* instance);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_LISTENSERVICE_INTERNAL_HEADER_GUARD*/
