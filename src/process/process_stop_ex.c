/**
 * \file src/process_stop_ex.c
 *
 * \brief Stop a process with options
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/process.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * \brief Stop a process, passing extended options.
 *
 * \param proc              The process to stop.
 * \param options           The options to send to waitpid.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success;
 *          - AGENTD_ERROR_PROCESS_NOT_ACTIVE if the process is not running.
 *          - An error code on failure.
 */
int process_stop_ex(process_t* proc, int options)
{
    MODEL_ASSERT(NULL != proc);

    /* can't stop a process that isn't running. */
    if (!proc->running)
    {
        return AGENTD_ERROR_PROCESS_NOT_ACTIVE;
    }

    /* send a terminate signal to the process. */
    kill(proc->process_id, SIGTERM);

    /* wait on this process to terminate. */
    int status;
    waitpid(proc->process_id, &status, options);

    /* if the options were 0, then we waited for the process to end. */
    if (0 == options)
    {
        /* update the running state to show that this process is not running. */
        proc->running = false;
    }

    /* success */
    return AGENTD_STATUS_SUCCESS;
}
