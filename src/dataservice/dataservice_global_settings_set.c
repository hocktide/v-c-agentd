/**
 * \file dataservice/dataservice_global_settings_set.c
 *
 * \brief Set a value in global settings.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Set a global setting via the dataservice_global_setting_enum
 * enumeration.
 *
 * \param ctx           The child context from which the global setting is
 *                      set.
 * \param key           The key value to set.
 * \param buffer        The buffer from which the setting is set.
 * \param size          The size of the buffer.
 *
 * \returns A status code indicating success or failure.
 *          - 0 on success
 *          - non-zero on failure.
 */
int dataservice_global_settings_set(
    dataservice_child_context_t* child, uint64_t key, const char* buffer,
    size_t size)
{
    int retval = 0;
    MDB_txn* txn;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);
    MODEL_ASSERT(NULL != buffer);
    MODEL_ASSERT(size > 0);
    MODEL_ASSERT(key >= DATASERVICE_GLOBAL_SETTING_LOWER_BOUND &&
        key < DATASERVICE_GLOBAL_SETTING_UPPER_BOUND);

    /* verify that we are allowed to set global settings. */
    if (!BITCAP_ISSET(child->childcaps,
            DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE))
    {
        retval = 3;
        goto done;
    }

    /* get the details for this database connection. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)child->root->details;

    /* create a write transaction for writing data to the database. */
    if (0 != mdb_txn_begin(details->env, NULL, 0, &txn))
    {
        retval = 4;
        goto done;
    }

    /* set up the key and val. */
    MDB_val lkey;
    lkey.mv_size = sizeof(key);
    lkey.mv_data = &key;
    MDB_val lval;
    lval.mv_size = size;
    lval.mv_data = (void*)buffer;

    /* attempt to put the value into the database. */
    if (0 != mdb_put(txn, details->global_db, &lkey, &lval, 0))
    {
        retval = 5;
        goto transaction_rollback;
    }

    /* attempt to commit the transaction. */
    if (0 != mdb_txn_commit(txn))
    {
        retval = 6;
        goto transaction_rollback;
    }

    /* success. */
    retval = 0;
    goto done;

transaction_rollback:
    mdb_txn_abort(txn);

done:
    return retval;
}
