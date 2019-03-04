/**
 * \file src/process_start.c
 *
 * \brief Start a process.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/process.h>

/**
 * \brief Start a process.
 *
 * \param proc              The process to start.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success;
 *          - AGENTD_ERROR_PROCESS_ALREADY_SPAWNED if the process was already
 *            started.
 *          - An error code on failure.
 */
int process_start(process_t* proc)
{
    MODEL_ASSERT(NULL != proc);

    /* only spawn once. */
    if (proc->running)
    {
        return AGENTD_ERROR_PROCESS_ALREADY_SPAWNED;
    }

    /* attempt to spawn the process, using the start method. */
    int retval = proc->init_method(proc);
    if (retval != AGENTD_STATUS_SUCCESS)
    {
        return retval;
    }

    /* update status to show that the process is running. */
    proc->running = true;

    /* success */
    return AGENTD_STATUS_SUCCESS;
}
