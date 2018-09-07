/**
 * \file agentd/supervisor.h
 *
 * \brief Supervisor functions.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_SUPERVISOR_HEADER_GUARD
#define AGENTD_SUPERVISOR_HEADER_GUARD

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

#include <agentd/bootstrap_config.h>

int supervisor_proc(struct bootstrap_config* bconf, int pid_fd);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_SUPERVISOR_HEADER_GUARD*/
