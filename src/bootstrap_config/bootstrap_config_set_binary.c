/**
 * \file bootstrap_config/bootstrap_config_set_binary.c
 *
 * \brief Set the binary name for this structure.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <agentd/path.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <vpr/parameters.h>

/**
 * \brief Set the binary name for agentd.
 *
 * The string value provided, typically as argv[0], is canonicalized and used as
 * the basis of the real path to this binary.  On success, the binary name is
 * updated.  On failure, this method returns a non-zero status.  The caller
 * retains ownership of the bname parameter; this method makes an internal copy
 * of the canonical name, which is freed when this bootstrap config structure is
 * dispose()d.
 *
 * \param bconf         The bootstrap configuration data to update.
 * \param bname         The name of the binary to be canonicalized and set.
 */
int bootstrap_config_set_binary(
    bootstrap_config_t* bconf, const char* bname)
{
    int retval = 1;
    const char* pathenv = getenv("PATH");
    char* path = NULL;

    MODEL_ASSERT(NULL != bconf);
    MODEL_ASSERT(NULL != bname);
    MODEL_ASSERT(NULL == bconf->binary);

    /* attempt to get the complete path searched for an executable. */
    if (0 != path_append_default(pathenv, &path))
    {
        retval = 1;
        goto done;
    }

    /* attempt to resolve this binary name. */
    if (0 != path_resolve(bname, path, (char**)&bconf->binary))
    {
        retval = 2;
        goto cleanup_path;
    }

    /* success. */
    retval = 0;

cleanup_path:
    free(path);

done:
    return retval;
}
