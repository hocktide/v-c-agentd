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
    int protosock;
} listenservice_instance_t;

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_LISTENSERVICE_INTERNAL_HEADER_GUARD*/
