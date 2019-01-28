/**
 * \file dataservice/dataservice_database_close.c
 *
 * \brief Close the lmdb database.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Close the database.
 *
 * \param ctx       The root context with the opened database.
 */
void dataservice_database_close(
    dataservice_root_context_t* ctx)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);

    /* get the database details. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx->details;

    /* force sync the database. */
    mdb_env_sync(details->env, 1);

    /* close dbi handles. */
    mdb_dbi_close(details->env, details->global_db);
    mdb_dbi_close(details->env, details->block_db);
    mdb_dbi_close(details->env, details->txn_db);
    mdb_dbi_close(details->env, details->pq_db);
    mdb_dbi_close(details->env, details->artifact_db);
    mdb_dbi_close(details->env, details->height_db);

    /* close database environment. */
    mdb_env_close(details->env);

    /* release details. */
    memset(details, 0, sizeof(dataservice_database_details_t));
    free(details);
}
