/**
 * \file dataservice/dataservice_database_open.c
 *
 * \brief Open the lmdb database.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

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
 * \returns 0 on success and non-zero on failure.
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
        retval = 1;
        goto done;
    }

    /* create the environment. */
    if (0 != mdb_env_create(&details->env))
    {
        retval = 2;
        goto free_details;
    }

    /* TODO - expose setting below as a param instead of a hard-coded value. */

    /* set the database size to an arbitrarily large value. */
    if (0 != mdb_env_set_mapsize(details->env, 8UL * 1024UL * 1024UL * 1024UL))
    {
        retval = 3;
        goto close_environment;
    }

    /* We need 4 database handles. */
    if (0 != mdb_env_set_maxdbs(details->env, 4))
    {
        retval = 4;
        goto close_environment;
    }

    /* open the environment. */
    if (0 != mdb_env_open(details->env, datadir, 0, 0600))
    {
        retval = 5;
        goto close_environment;
    }

    /* create a transaction for opening the databases. */
    if (0 != mdb_txn_begin(details->env, NULL, 0, &txn))
    {
        retval = 6;
        goto close_environment;
    }

    /* open the global database. */
    if (0 != mdb_dbi_open(txn, "global.db", MDB_CREATE, &details->global_db))
    {
        retval = 7;
        goto rollback_txn;
    }

    /* open the block database. */
    if (0 != mdb_dbi_open(txn, "block.db", MDB_CREATE, &details->block_db))
    {
        retval = 8;
        goto rollback_txn;
    }

    /* open the txn database. */
    if (0 != mdb_dbi_open(txn, "txn.db", MDB_CREATE, &details->txn_db))
    {
        retval = 9;
        goto rollback_txn;
    }

    /* open the process queue database. */
    if (0 != mdb_dbi_open(txn, "pq.db", MDB_CREATE, &details->pq_db))
    {
        retval = 10;
        goto rollback_txn;
    }

    /* commit the open. */
    if (0 != mdb_txn_commit(txn))
    {
        retval = 11;
        goto rollback_txn;
    }

    /* success. */
    retval = 0;
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
