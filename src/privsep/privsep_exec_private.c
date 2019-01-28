/**
 * \file privsep/privsep_exec_private.c
 *
 * \brief Execute a private command.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/privsep.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

/**
 * \brief Execute a private command.
 *
 * \param command       The private command to execute.
 *
 * \returns An error code on failure.  This method not return on success;
 * instead, the process is replaced.
 *      - AGENTD_ERROR_GENERAL_PRIVSEP_EXEC_PRIVATE_SETENV_FAILURE is returned
 *        when attempting to set the PATH / LD_LIBRARY_PATH variables fails.
 *      - AGENTD_ERROR_GENERAL_PRIVSEP_EXEC_PRIVATE_EXECL_FAILURE is returned
 *        when the execl call fails to start the private command.
 */
int privsep_exec_private(const char* command)
{
    MODEL_ASSERT(NULL != command);

    /* set PATH */
    if (0 != setenv("PATH", "/bin", 1))
    {
        return AGENTD_ERROR_GENERAL_PRIVSEP_EXEC_PRIVATE_SETENV_FAILURE;
    }

    /* set LD_LIBRARY_PATH */
    if (0 != setenv("LD_LIBRARY_PATH", "/lib:/usr/libexec", 1))
    {
        return AGENTD_ERROR_GENERAL_PRIVSEP_EXEC_PRIVATE_SETENV_FAILURE;
    }

    /* spawn the child process (this does not return if successful. */
    if (0 != execl("/bin/agentd", "agentd", "-P", command, NULL))
    {
        return AGENTD_ERROR_GENERAL_PRIVSEP_EXEC_PRIVATE_EXECL_FAILURE;
    }

    /* success. This line will never execute, but it is needed for compiler
     * checks. */
    return AGENTD_STATUS_SUCCESS;
}
