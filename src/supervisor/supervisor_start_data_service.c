/**
 * \file supervisor/supervisor_start_data_service.c
 *
 * \brief Start the data service process.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/dataservice.h>
#include <agentd/dataservice/api.h>
#include <unistd.h>

#include "supervisor_private.h"

/**
 * \brief Start the data service.
 *
 * \param proc      The data service to start.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 */
int supervisor_start_data_service(process_t* proc)
{
    dataservice_process_t* data_proc = (dataservice_process_t*)proc;
    int retval;
    uint32_t offset, status;

    /* attempt to create the data service. */
    TRY_OR_FAIL(
        dataservice_proc(
            data_proc->bconf, data_proc->conf, *data_proc->log_socket,
            data_proc->supervisor_data_socket, &data_proc->hdr.process_id,
            true),
        done);

    /* attempt to send the initialize root context request. */
    TRY_OR_FAIL(
        dataservice_api_sendreq_root_context_init_block(
            *data_proc->supervisor_data_socket, data_proc->conf->datastore),
        terminate_proc);

    /* attempt to read the response from this init. */
    TRY_OR_FAIL(
        dataservice_api_recvresp_root_context_init_block(
            *data_proc->supervisor_data_socket, &offset, &status),
        terminate_proc);

    /* verify that the operation completed successfully. */
    TRY_OR_FAIL(status, terminate_proc);

    /* attempt to reduce the root capabilities. */
    TRY_OR_FAIL(
        dataservice_api_sendreq_root_context_reduce_caps_block(
            *data_proc->supervisor_data_socket,
            data_proc->reducedcaps, sizeof(data_proc->reducedcaps)),
        terminate_proc);

    /* attempt to read the response from this operation. */
    TRY_OR_FAIL(
        dataservice_api_recvresp_root_context_reduce_caps_block(
            *data_proc->supervisor_data_socket, &offset, &status),
        terminate_proc);

    /* verify that the operation completed successfully. */
    TRY_OR_FAIL(status, terminate_proc);

    /* if successful, the child process owns the sockets. */
    *data_proc->log_socket = -1;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

terminate_proc:
    /* force the running status to true so we can terminate the process. */
    data_proc->hdr.running = true;
    process_stop((process_t*)data_proc);

done:
    return retval;
}
