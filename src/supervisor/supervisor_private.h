/**
 * \file supervisor/supervisor_private.h
 *
 * \brief Private supervisor functions for setting up services.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_SUPERVISOR_PRIVATE_HEADER_GUARD
#define AGENTD_SUPERVISOR_PRIVATE_HEADER_GUARD

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include <agentd/dataservice.h>
#include <agentd/supervisor.h>
#include <agentd/supervisor/supervisor_internal.h>

/**
 * \brief Data service process structure.
 */
typedef struct dataservice_process
{
    process_t hdr;
    const bootstrap_config_t* bconf;
    const agent_config_t* conf;
    int* supervisor_data_socket;
    int* log_socket;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);
} dataservice_process_t;

/**
 * \brief Dispose of the data service.
 *
 * \param disposable        The data service process to clean up.
 */
void supervisor_dispose_data_service(void* disposable);

/**
 * \brief Start the data service.
 *
 * \param proc      The data service to start.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 */
int supervisor_start_data_service(process_t* proc);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*AGENTD_SUPERVISOR_PRIVATE_HEADER_GUARD*/
