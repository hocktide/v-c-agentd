/**
 * \file dataservice/dataservice_root_context_init.c
 *
 * \brief Initialize the root context for the data service.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/* forward decls */
static void dataservice_root_context_dispose(void* disposable);

/**
 * \brief Create a root data service context.
 *
 * \param ctx           The private data service context to initialize.
 * \param datadir       The data directory for this private data service.
 *
 * \returns 0 on success and non-zero on failure.
 */
int dataservice_root_context_init(
    dataservice_root_context_t* ctx, const char* datadir)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(NULL != datadir);

    /* verify that we are allowed to create a root context. */
    if (!BITCAP_ISSET(ctx->apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE))
        return 1;

    /* clear the context. */
    memset(ctx, 0, sizeof(dataservice_root_context_t));

    /* initialize the root capabilities. By default, all capabilities are
     * granted, except the capability to create a new root context. */
    BITCAP_INIT_TRUE(ctx->apicaps);
    BITCAP_SET_FALSE(ctx->apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* this context can be disposed. */
    ctx->hdr.dispose = &dataservice_root_context_dispose;

    /* attempt to open the database and forward status to the caller. */
    return dataservice_database_open(ctx, datadir);
}

/**
 * \brief Dispose of the root data service context.
 *
 * \param disposable        The root data service to dispose.
 */
static void dataservice_root_context_dispose(void* disposable)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != disposable);

    /* this is a root context. */
    dataservice_root_context_t* ctx = (dataservice_root_context_t*)disposable;

    /* close the database. */
    dataservice_database_close(ctx);

    /* clear this structure. */
    memset(ctx, 0, sizeof(dataservice_root_context_t));
}
