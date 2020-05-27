/**
 * \file src/process_stop.c
 *
 * \brief Stop a process.
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/process.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * \brief Stop a process.
 *
 * \param proc              The process to stop.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success;
 *          - AGENTD_ERROR_PROCESS_NOT_ACTIVE if the process is not running.
 *          - An error code on failure.
 */
int process_stop(process_t* proc)
{
    MODEL_ASSERT(NULL != proc);

    return process_stop_ex(proc, WNOWAIT);
}
