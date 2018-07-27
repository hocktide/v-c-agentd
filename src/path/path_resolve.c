/**
 * \file path/path_resolve.c
 *
 * \brief Attempt to resolve a file name to a pathname.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/string.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

/* forward decls */
static int valid_executable(const char* filename, char** fullpath);

/**
 * \brief Given a filename and a path, attempt to resolve the filename using
 * this path.
 *
 * On success, resolved is updated to the resolved filename.  This value is
 * owned by the caller and must be free()d when no longer needed.
 *
 * \param filename          The filename to resolve.
 * \param path              A colon separated list of path values.
 * \param resolved          The character pointer that this parameter points to
 *                          is updated to the resolved path on success.
 *
 * \returns 0 on success and non-zero on failure.
 */
int path_resolve(const char* filename, const char* path, char** resolved)
{
    int retval = 1;
    char *workpath = NULL, *pathent = NULL, *pathlast = NULL;

    MODEL_ASSERT(NULL != filename);
    MODEL_ASSERT(NULL != path);
    MODEL_ASSERT(NULL != resolved);

    /* start by setting resolved to NULL. */
    *resolved = NULL;

    /* attempt to canonicalize this as a valid executable. */
    if (0 == valid_executable(filename, resolved))
    {
        retval = 0;
        goto done;
    }

    /* duplicate the working path so it can be manipulated by strtok. */
    workpath = strdup(path);
    if (NULL == workpath)
    {
        retval = 1;
        goto done;
    }

    /* iterate through path entries */
    for (
        /* initialize pathent with the first path entry. */
        pathent = strtok_r(workpath, ":", &pathlast);
        /* continue while there are path entries. */
        NULL != pathent;
        /* get the next entry. */
        pathent = strtok_r(NULL, ":", &pathlast))
    {
        /* build a path using the current path entry and the filename. */
        char* fullpath = strcatv(pathent, "/", filename, NULL);
        if (NULL == fullpath)
        {
            retval = 1;
            goto cleanup_workpath;
        }

        /* check to see if this is a valid executable. */
        retval = valid_executable(fullpath, resolved);

        /* clean up fullpath. */
        free(fullpath);

        /* if this is a valid executable, exit the loop and clean up. */
        if (0 == retval)
        {
            goto cleanup_workpath;
        }
    }

    /* no valid executable was found. */
    retval = 1;

cleanup_workpath:
    if (NULL != workpath)
    {
        free(workpath);
    }

done:
    return retval;
}

/**
 * \brief Test whether the given executable is valid.  If so, this method
 * succeeds and \ref fullpath is set to a heap allocated real path to this
 * executable.  On success, the string set in fullpath is owned by the caller
 * and must be free()d when no longer needed.
 *
 * \param filename          The name of the file to test.
 * \param fullpath          A pointer to a character pointer that is updated
 *                          with the full path to the executable on success.
 *
 * \returns 0 on success and non-zero on failure.
 */
static int valid_executable(const char* filename, char** fullpath)
{
    /* attempt to get the real path to this filename. */
    *fullpath = realpath(filename, NULL);
    if (NULL == *fullpath)
    {
        return 1;
    }

    /* verify that this filename exists and is executable. */
    if (0 != access(*fullpath, X_OK))
    {
        free(*fullpath);
        *fullpath = NULL;
        return 1;
    }

    /* success. */
    return 0;
}
