/**
 * \file dataservice/dataservice_root_context_init.c
 *
 * \brief Initialize the root context for the data service.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
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
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if the root context is not
 *        authorized to perform this action.
 *      - AGENTD_ERROR_DATASERVICE_MDB_ENV_CREATE_FAILURE if this function
 *        failed to create a database environment.
 *      - AGENTD_ERROR_DATASERVICE_MDB_ENV_SET_MAPSIZE_FAILURE if this function
 *        failed to set the database map size.
 *      - AGENTD_ERROR_DATASERVICE_MDB_ENV_SET_MAXDBS_FAILURE if this function
 *        failed to set the maximum number of databases.
 *      - AGENTD_ERROR_DATASERVICE_MDB_ENV_OPEN_FAILURE if this function failed
 *        to open the database environment.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_DBI_OPEN_FAILURE if this function failed
 *        to open a database instance.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_COMMIT_FAILURE if this function
 *        failed to commit the database open transaction.
 */
int dataservice_root_context_init(
    dataservice_root_context_t* ctx, const char* datadir)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(NULL != datadir);

    /* verify that we are allowed to create a root context. */
    if (!BITCAP_ISSET(ctx->apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE))
        return AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED;

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
