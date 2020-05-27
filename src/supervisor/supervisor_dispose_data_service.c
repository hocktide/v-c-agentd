/**
 * \file supervisor/supervisor_dispose_data_service.c
 *
 * \brief Dispose the data service process.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <unistd.h>

#include "supervisor_private.h"

/**
 * \brief Dispose of the data service.
 *
 * \param disposable        The data service process to clean up.
 */
void supervisor_dispose_data_service(void* disposable)
{
    dataservice_process_t* data_proc = (dataservice_process_t*)disposable;

    /* clean up the supervisor socket if valid. */
    if (data_proc->supervisor_data_socket)
    {
        close(*data_proc->supervisor_data_socket);
        *data_proc->supervisor_data_socket = -1;
    }

    /* clean up the log socket if valid. */
    if (*data_proc->log_socket > 0)
    {
        close(*data_proc->log_socket);
        *data_proc->log_socket = -1;
    }

    /* if the process is running, stop and then kill it. */
    if (data_proc->hdr.running)
    {
        /* call the process stop method. */
        process_stop((process_t*)data_proc);

        sleep(5);

        /* call the process kill method. */
        process_kill((process_t*)data_proc);
    }
}
