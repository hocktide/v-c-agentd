/**
 * \file dataservice/dataservice_internal.h
 *
 * \brief Internal header for the data service.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_DATASERVICE_INTERNAL_HEADER_GUARD
#define AGENTD_DATASERVICE_INTERNAL_HEADER_GUARD

#include <agentd/dataservice/private/dataservice.h>
#include <lmdb.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief The database details structure used to maintain a database connection.
 */
typedef struct dataservice_database_details
{
    MDB_env* env;
    MDB_dbi global_db;
    MDB_dbi block_db;
    MDB_dbi txn_db;
    MDB_dbi pq_db;
} dataservice_database_details_t;

/**
 * \brief Open the database using the given data directory.
 *
 * \param ctx       The initialized root context that stores this database.
 * \param datadir   The directory where the database is stored.
 *
 * \returns 0 on success and non-zero on failure.
 */
int dataservice_database_open(
    dataservice_root_context_t* ctx, const char* datadir);

/**
 * \brief Close the database.
 *
 * \param ctx       The root context with the opened database.
 *
 * \returns 0 on success and non-zero on failure.
 */
void dataservice_database_close(
    dataservice_root_context_t* ctx);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_INTERNAL_HEADER_GUARD*/
