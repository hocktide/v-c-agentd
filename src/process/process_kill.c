/**
 * \file src/process_kill.c
 *
 * \brief Kill a process.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/process.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * \brief Kill a process.
 *
 * \param proc              The process to kill.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success;
 *          - AGENTD_ERROR_PROCESS_NOT_ACTIVE if the process is not running.
 *          - An error code on failure.
 */
int process_kill(process_t* proc)
{
    MODEL_ASSERT(NULL != proc);

    /* can't kill a process that isn't running. */
    if (!proc->running)
    {
        return AGENTD_ERROR_PROCESS_NOT_ACTIVE;
    }

    /* send a terminate signal to the process. */
    kill(proc->process_id, SIGKILL);

    /* wait on this process to terminate. */
    int status;
    waitpid(proc->process_id, &status, 0);

    /* update the running state to show that this process is not running. */
    proc->running = false;

    /* success */
    return AGENTD_STATUS_SUCCESS;
}
