/**
 * \file commandline/commandline_print_usage.c
 *
 * \brief Print the commandline usage and return the given return code.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/commandline.h>
#include <cbmc/model_assert.h>

/**
 * \brief Print usage and return the given response code.
 *
 * \param out           The file handle to use for output.
 * \param returncode    The return code to return to the caller.
 */
int commandline_print_usage(FILE* out, int returncode)
{
    MODEL_ASSERT(NULL != out);

    fprintf(out, "Usage: agentd [-F] command\n\n");
    fprintf(out, "Where:\n");
    fprintf(out, "\t\t-F         \tRun in foreground (non-daemon mode).\n");
    fprintf(out, "\n");
    fprintf(out, "supported commands:\n");
    fprintf(out, "\t\thelp       \tPrint this help info.\n");
    fprintf(out, "\t\treadconfig \tRead the config file and display settings.\n");

    return returncode;
}
