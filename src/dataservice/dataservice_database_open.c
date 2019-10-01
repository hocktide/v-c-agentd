/**
 * \file dataservice/dataservice_database_open.c
 *
 * \brief Open the lmdb database.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Open the database using the given data directory.
 *
 * \param ctx       The initialized root context that stores this database.
 * \param datadir   The directory where the database is stored.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
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
int dataservice_database_open(
    dataservice_root_context_t* ctx, const char* datadir)
{
    int retval = 0;
    MDB_txn* txn;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != ctx);
    MODEL_ASSERT(NULL != datadir);

    /* attempt to allocate memory for the database details structure. */
    ctx->details = malloc(sizeof(dataservice_database_details_t));
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx->details;
    if (NULL == details)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* create the environment. */
    if (0 != mdb_env_create(&details->env))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_ENV_CREATE_FAILURE;
        goto free_details;
    }

    /* TODO - expose setting below as a param instead of a hard-coded value. */

    /* set the database size to an arbitrarily large value. */
    if (0 != mdb_env_set_mapsize(details->env, 8UL * 1024UL * 1024UL * 1024UL))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_ENV_SET_MAPSIZE_FAILURE;
        goto close_environment;
    }

    /* We need 6 database handles. */
    if (0 != mdb_env_set_maxdbs(details->env, 6))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_ENV_SET_MAXDBS_FAILURE;
        goto close_environment;
    }

    /* open the environment. */
    if (0 != mdb_env_open(details->env, datadir, 0, 0600))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_ENV_OPEN_FAILURE;
        goto close_environment;
    }

    /* create a transaction for opening the databases. */
    if (0 != mdb_txn_begin(details->env, NULL, 0, &txn))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE;
        goto close_environment;
    }

    /* open the global database. */
    if (0 != mdb_dbi_open(txn, "global.db", MDB_CREATE, &details->global_db))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_DBI_OPEN_FAILURE;
        goto rollback_txn;
    }

    /* open the block database. */
    if (0 != mdb_dbi_open(txn, "block.db", MDB_CREATE, &details->block_db))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_DBI_OPEN_FAILURE;
        goto rollback_txn;
    }

    /* open the txn database. */
    if (0 != mdb_dbi_open(txn, "txn.db", MDB_CREATE, &details->txn_db))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_DBI_OPEN_FAILURE;
        goto rollback_txn;
    }

    /* open the process queue database. */
    if (0 != mdb_dbi_open(txn, "pq.db", MDB_CREATE, &details->pq_db))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_DBI_OPEN_FAILURE;
        goto rollback_txn;
    }

    /* open the artifact database. */
    if (0 != mdb_dbi_open(txn, "artifact.db", MDB_CREATE, &details->artifact_db))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_DBI_OPEN_FAILURE;
        goto rollback_txn;
    }

    /* open the block height database. */
    if (0 != mdb_dbi_open(txn, "height.db", MDB_CREATE, &details->height_db))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_DBI_OPEN_FAILURE;
        goto rollback_txn;
    }

    /* commit the open. */
    if (0 != mdb_txn_commit(txn))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_TXN_COMMIT_FAILURE;
        goto close_environment;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

rollback_txn:
    mdb_txn_abort(txn);

close_environment:
    mdb_env_close(details->env);

free_details:
    free(details);

done:
    return retval;
}
