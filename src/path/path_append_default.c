/**
 * \file path/path_append_default.c
 *
 * \brief Append the default path onto the given path.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <cbmc/model_assert.h>
#include <paths.h>
#include <string.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Append the default path onto a given path.
 *
 * On success, outpath is updated to the appended path.  This value is owned by
 * the caller and must be free()d when no longer needed.
 *
 * \param path              The path to append.
 * \param outpath           The character pointer that this parameter points to
 *                          is updated to the appended path on success.
 *
 * \returns 0 on success and non-zero on failure.
 */
int path_append_default(const char* path, char** outpath)
{
    MODEL_ASSERT(NULL != path);
    MODEL_ASSERT(NULL != outpath);

    size_t pathlen = strlen(path);

    /* if the path is empty, return a copy of the default path. */
    if (!strlen(path))
    {
        *outpath = strdup(_PATH_DEFPATH);
    }
    else
    {
        size_t defpathlen = strlen(_PATH_DEFPATH);
        size_t total_pathlen = pathlen + defpathlen + 2;

        /* allocate memory for the appended path. */
        *outpath = (char*)malloc(total_pathlen);
        if (NULL == *outpath)
            return 1;

        /* construct the appended path. */
        memcpy((*outpath) + 0, path, pathlen);
        memcpy((*outpath) + pathlen, ":", 1);
        memcpy((*outpath) + pathlen + 1, _PATH_DEFPATH, defpathlen);
        (*outpath)[pathlen + 1 + defpathlen] = 0;
    }

    return 0;
}
