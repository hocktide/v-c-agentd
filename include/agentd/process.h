/**
 * \file agentd/process.h
 *
 * \brief Process management.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_PROCESS_HEADER_GUARD
#define AGENTD_PROCESS_HEADER_GUARD

#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/* forward decl for process. */
struct process;

/**
 * \brief A process start method takes a process structure and populates a
 * process id pointer associated with the created process.
 *
 * \param proc              The process_t structure to use for initializing the
 *                          process.
 * \param pid               Pointer to the pid_t used to hold the new process
 *                          ID.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - An error code on failure.
 */
typedef int (*process_start_fn_t)(struct process* proc);

/**
 * \brief The process structure manages the lifecycle of a child process.
 *
 * The child process can be started and stopped.  There is a process ID
 * associated with the process.  When this process structure is disposed, an
 * attempt is made to stop the process if running.
 */
typedef struct process
{
    disposable_t hdr;
    process_start_fn_t init_method;
    pid_t process_id;
    bool running;
} process_t;

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
int process_start(process_t* proc);

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
int process_stop(process_t* proc);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_PROCESS_HEADER_GUARD*/
