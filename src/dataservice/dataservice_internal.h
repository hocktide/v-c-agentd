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
#include <event.h>
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
 * \brief Child details.
 */
typedef struct dataservice_child_details
{
    disposable_t hdr;
    struct dataservice_child_details* next;
    dataservice_child_context_t ctx;
} dataservice_child_details_t;

/**
 * \brief Support up to 1024 child contexts.
 */
#define DATASERVICE_MAX_CHILD_CONTEXTS 1024

/**
 * \brief The database service instance.
 */
typedef struct dataservice_instance
{
    disposable_t hdr;
    dataservice_root_context_t ctx;
    dataservice_child_details_t children[DATASERVICE_MAX_CHILD_CONTEXTS];
    dataservice_child_details_t* child_head;
} dataservice_instance_t;

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

/**
 * \brief Create the dataservice instance.
 *
 * \returns a properly created dataservice instance, or NULL on failure.
 */
dataservice_instance_t* dataservice_instance_create();

/**
 * \brief Create a child details structure for the given dataservice instance.
 *
 * \param inst          The instance in which this child context is created.
 * \param offset        Pointer to the offset that is updated with this child
 *                      context offset in the children array.
 *
 * \returns 0 on success, and non-zero on failure.
 */
int dataservice_child_details_create(dataservice_instance_t* inst, int* offset);

/**
 * \brief Reclaim a child details structure.
 *
 * \param inst          The instance to which this structure belongs.
 * \param offset        The offset to reclaim.
 */
void dataservice_child_details_delete(dataservice_instance_t* inst, int offset);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_INTERNAL_HEADER_GUARD*/
