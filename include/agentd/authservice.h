/**
 * \file agentd/authservice.h
 *
 * \brief Service level API for the auth service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_AUTHSERVICE_HEADER_GUARD
#define AGENTD_AUTHSERVICE_HEADER_GUARD

#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus


/**
 * \brief Event loop for the authentication service.  This is the entry point 
 * for the auth service.  It handles the details of reacting to events
 * sent over the auth service socket.
 *
 * \param authsock      The auth service socket.  The auth service listens for 
 *                      connections on this socket.
 * \param logsock       The logging service socket.  The auth service logs on 
 *                      this socket.
 *
 * \returns a status code on service exit indicating a normal or abnormal exit.
 *          - AGENTD_STATUS_SUCCESS on normal exit.
 *          - AGENTD_ERROR_AUTHSERVICE_IPC_MAKE_NOBLOCK_FAILURE if
 *            attempting to make the process socket non-blocking failed.
 *          - AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            initializing the event loop failed.
 *          - AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_ADD_FAILURE if adding
 *            the auth service socket to the event loop failed.
 *          - AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_RUN_FAILURE if running
 *            the auth service event loop failed.
 */
int auth_service_event_loop(int authsock, int logsock);


/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_AUTHSERVICE_HEADER_GUARD*/
