/**
 * \file path/path_append_default.c
 *
 * \brief Append the default path onto the given path.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/string.h>
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

    /* if the path is empty, return a copy of the default path. */
    if (0 == path[0])
    {
        *outpath = strdup(_PATH_DEFPATH);
    }
    else
    {
        *outpath = strcatv(path, ":", _PATH_DEFPATH, NULL);
        if (NULL == *outpath)
            return 1;
    }

    return 0;
}
