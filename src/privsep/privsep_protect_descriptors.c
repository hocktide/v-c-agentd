/**
 * \file privsep/privsep_protect_descriptors.c
 *
 * \brief Protect file descriptors by moving them out of the way.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/privsep.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <unistd.h>

/**
 * \brief Make sure file descriptors aren't standard file descriptors; if they
 * are, move them out of the way.
 *
 * This function takes pointers to descriptors and expects this list to be
 * terminated by NULL.
 *
 * \param desc          Pointer to a descriptor to check and possibly move.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_DUP2_FAILURE if setting a file
 *            descriptor fails.
 *          - AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_BAD_ARGUMENT if a bad argument
 *            is encountered.
 */
int privsep_protect_descriptors(int* desc, ...)
{
    int retval = 1;
    va_list fd_list;

    /* start the fd list. */
    va_start(fd_list, desc);

    int high_desc = 500;

    /* replace each descriptor. */
    for (;;)
    {
        /* map the file descriptor to a high value. */
        retval = dup2(*desc, high_desc);
        if (retval < 0)
        {
            retval = AGENTD_ERROR_GENERAL_PRIVSEP_SETFDS_DUP2_FAILURE;
            goto done;
        }

        /* close the old fd. */
        close(*desc);

        /* copy over the descriptor. */
        *desc = high_desc++;

        /* attempt to read the next descriptor. */
        desc = va_arg(fd_list, int*);
        if (NULL == desc)
        {
            retval = AGENTD_STATUS_SUCCESS;
            goto done;
        }
    }

    /* success */
    retval = AGENTD_STATUS_SUCCESS;

done:
    va_end(fd_list);

    return retval;
}
