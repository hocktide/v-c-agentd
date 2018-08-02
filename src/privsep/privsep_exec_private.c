/**
 * \file privsep/privsep_exec_private.c
 *
 * \brief Execute a private command.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/privsep.h>
#include <cbmc/model_assert.h>
#include <unistd.h>

/**
 * \brief Execute a private command.
 *
 * \param command       The private command to execute.
 *
 * \returns non-zero on failure.  Does not return on success; process is
 * replaced.
 */
int privsep_exec_private(const char* command)
{
    int retval = 1;

    MODEL_ASSERT(NULL != command);

    /* set PATH */
    retval = setenv("PATH", "/bin", 1);
    if (0 != retval)
    {
        return retval;
    }

    /* set LD_LIBRARY_PATH */
    retval = setenv("LD_LIBRARY_PATH", "/lib:/usr/libexec", 1);
    if (0 != retval)
    {
        return retval;
    }

    /* spawn the child process (this does not return if successful. */
    return execl("/bin/agentd", "agentd", "-P", command, NULL);
}
