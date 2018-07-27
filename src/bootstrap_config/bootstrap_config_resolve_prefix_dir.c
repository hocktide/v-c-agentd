/**
 * \file bootstrap_config/bootstrap_config_resolve_prefix_dir.c
 *
 * \brief Resolve the installation prefix to which a given binary was installed.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/bootstrap_config.h>
#include <agentd/path.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <vpr/parameters.h>

/**
 * \brief Resolve the prefix directory using the binary name.
 *
 * This method can only be called after \ref bootstrap_config_set_binary() was
 * called and returned a successful status code.  This method sets the prefix
 * directory based on the binary name.  This prefix directory is used for
 * creating a temporary \ref chroot() so the config file can be read.
 *
 * \param bconf         The bootstrap configuration data to update.
 */
int bootstrap_config_resolve_prefix_dir(
    bootstrap_config_t* bconf)
{
    char* bindir = NULL;
    MODEL_ASSERT(NULL != bconf);
    MODEL_ASSERT(NULL != bconf->binary);

    /* if the binary is NULL, then it don't attempt to resolve it.  * */
    if (NULL == bconf->binary)
    {
        return 1;
    }

    /* get the directory for this binary. */
    if (0 != path_dirname(bconf->binary, &bindir))
    {
        return 1;
    }

    /* get the directory that this binary directory is in. */
    int retval = path_dirname(bindir, (char**)&bconf->prefix_dir);

    /* clean up. */
    free(bindir);

    return retval;
}
