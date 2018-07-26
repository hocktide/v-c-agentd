/**
 * \file path/path_dirname.c
 *
 * \brief Return the directory portion of the path.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Given a pathname, return the directory portion of this pathname.
 *
 * On success, dirname is updated to the directory portion of the filename.
 * This value is owned by the caller and must be free()d when no longer needed.
 *
 * \param filename          The filename to resolve.
 * \param dirname           The character pointer that this parameter points to
 *                          is updated to the directory name on success.
 *
 * \returns 0 on success and non-zero on failure.
 */
int path_dirname(const char* filename, char** dirname)
{
    int retval = 1;
    char* workpath = NULL;
    char* dirpath = NULL;
    char* pathlast = NULL;
    const char* dirsep = NULL;
    MODEL_ASSERT(NULL != dirname);

    /* set dirname to a sentry value. */
    *dirname = NULL;

    /* if filename is an empty string, then return the current directory. */
    if (NULL == filename || 0 == filename[0])
    {
        /* duplicate a string representation of the current directory. */
        *dirname = strdup(".");
        if (NULL == *dirname)
        {
            retval = 1;
            goto cleanup;
        }

        retval = 0;
    }
    else
    {
        /* copy the filename for the working path. */
        workpath = strdup(filename);
        if (NULL == workpath)
        {
            retval = 1;
            goto cleanup;
        }

        /* the maximum size of the path is the size of the filename. */
        size_t pathmax = strlen(filename);
        size_t pathrem = pathmax;
        *dirname = (char*)malloc(pathmax + 1);
        if (NULL == *dirname)
        {
            retval = 1;
            goto cleanup;
        }

        /* clear dirname. */
        (*dirname)[0] = 0;

        /* iterate through the directories, copying these to the string. */
        for (
            /* initialize pathent with the first path entry. */
            char* pathent = strtok_r(workpath, "/", &pathlast);
            /* continue while there are no more path entries. */
            NULL != pathent;
            /* get the next entry. */
            pathent = strtok_r(NULL, "/", &pathlast))
        {
            /* if dirpath exists, append it. */
            if (NULL != dirpath)
            {
                /* if dirsep exists, append it. */
                if (NULL != dirsep)
                {
                    strncat(*dirname, dirsep, pathrem);
                    pathrem -= strlen(dirsep);
                }
                else
                {
                    dirsep = "/";
                }

                /* append dirpath */
                strncat(*dirname, dirpath, pathrem);
                pathrem -= strlen(dirpath);
            }

            /* update dirpath */
            dirpath = pathent;
        }

        /* if we only encountered one entry or fewer, set *dirname to "." */
        if (NULL == dirsep)
        {
            strncpy(*dirname, ".", pathrem);
        }

        /* force dirname to be zero-terminated. */
        (*dirname)[pathmax] = 0;

        /* success. */
        retval = 0;
    }

cleanup:
    if (NULL != workpath)
    {
        free(workpath);
    }

    return retval;
}
