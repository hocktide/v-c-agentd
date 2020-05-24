/**
 * \file listenservice/listenservice_count_sockets.c
 *
 * \brief Count the number of sockets to which this service will listen.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "listenservice_internal.h"

/**
 * \brief Count the number of listen sockets, returning this number as an
 * integer.
 *
 * \param listenstart       The starting socket from which the count starts.
 *
 * \returns the number of valid descriptors found.
 */
int listenservice_count_sockets(int listenstart)
{
    int count = 0;
    int socket_good = 0;
    struct stat statbuf;

    /* iterate through each file descriptor. */
    do
    {
        /* get the status of the descriptor. */
        int retval = fstat(listenstart + count, &statbuf);
        if (retval < 0)
        {
            /* invalid descriptor.  We're done. */
            socket_good = 0;
        }
        else
        {
            /* valid descriptor.  Up count and continue. */
            ++count;
            socket_good = 1;
        }

    } while (socket_good);


    return count;
}
