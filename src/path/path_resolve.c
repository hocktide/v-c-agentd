/**
 * \file path/path_resolve.c
 *
 * \brief Attempt to resolve a file name to a pathname.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/status_codes.h>
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
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if the operation cannot
 *            be completed due to a memory allocation error.
 *          - AGENTD_ERROR_GENERAL_PATH_NOT_FOUND if the filename could not be
 *            found in the given path.
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
    if (AGENTD_STATUS_SUCCESS == valid_executable(filename, resolved))
    {
        retval = AGENTD_STATUS_SUCCESS;
        goto done;
    }

    /* duplicate the working path so it can be manipulated by strtok. */
    workpath = strdup(path);
    if (NULL == workpath)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
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
            retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
            goto cleanup_workpath;
        }

        /* check to see if this is a valid executable. */
        retval = valid_executable(fullpath, resolved);

        /* clean up fullpath. */
        free(fullpath);

        /* if this is a valid executable, exit the loop and clean up. */
        if (AGENTD_STATUS_SUCCESS == retval)
        {
            goto cleanup_workpath;
        }
    }

    /* no valid executable was found. */
    retval = AGENTD_ERROR_GENERAL_PATH_NOT_FOUND;

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
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_GENERAL_PATH_NOT_FOUND if the executable is not
 *            valid.
 */
static int valid_executable(const char* filename, char** fullpath)
{
    /* attempt to get the real path to this filename. */
    *fullpath = realpath(filename, NULL);
    if (NULL == *fullpath)
    {
        return AGENTD_ERROR_GENERAL_PATH_NOT_FOUND;
    }

    /* verify that this filename exists and is executable. */
    if (0 != access(*fullpath, X_OK))
    {
        free(*fullpath);
        *fullpath = NULL;
        return AGENTD_ERROR_GENERAL_PATH_NOT_FOUND;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
