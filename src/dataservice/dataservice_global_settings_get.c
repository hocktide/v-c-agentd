/**
 * \file dataservice/dataservice_global_settings_get.c
 *
 * \brief Get a value from the global settings.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Query a global setting via the dataservice_global_setting_enum
 * enumeration.
 *
 * \param ctx           The child context from which the global setting is
 *                      queried.
 * \param key           The key value to query.
 * \param buffer        The buffer to which the value is written.
 * \param size          The size of the buffer; updated to the size read if
 *                      successful or the size required if a would truncate
 *                      error occurs.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_DATASERVICE_NOT_FOUND if this global setting could not be
 *        found.
 *      - AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED if the child context is not
 *        authorized to call this function.
 *      - AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE if this function failed
 *        to begin a transaction.
 *      - AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE if this function failed to
 *        read from the database.
 *      - AGENTD_ERROR_DATASERVICE_WOULD_TRUNCATE if the provided buffer would
 *        truncate the value.  The size parameter is updated with the size of
 *        this value.
 */
int dataservice_global_settings_get(
    dataservice_child_context_t* child, uint64_t key, char* buffer,
    size_t* size)
{
    int retval = 0;
    MDB_txn* txn;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != child);
    MODEL_ASSERT(NULL != child->root);
    MODEL_ASSERT(NULL != buffer);
    MODEL_ASSERT(NULL != size);
    MODEL_ASSERT(key >= DATASERVICE_GLOBAL_SETTING_LOWER_BOUND &&
        key < DATASERVICE_GLOBAL_SETTING_UPPER_BOUND);
    MODEL_ASSERT(*size > 0);

    /* verify that we are allowed to query global settings. */
    if (!BITCAP_ISSET(child->childcaps,
            DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ))
    {
        retval = AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED;
        goto done;
    }

    /* get the details for this database connection. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)child->root->details;

    /* create a read transaction for reading data from the database. */
    if (0 != mdb_txn_begin(details->env, NULL, MDB_RDONLY, &txn))
    {
        retval = AGENTD_ERROR_DATASERVICE_MDB_TXN_BEGIN_FAILURE;
        goto done;
    }

    /* set up the key and val. */
    MDB_val lkey;
    lkey.mv_size = sizeof(key);
    lkey.mv_data = &key;
    MDB_val lval;
    memset(&lval, 0, sizeof(lval));

    /* attempt to get the value from the database. */
    retval = mdb_get(txn, details->global_db, &lkey, &lval);
    if (MDB_NOTFOUND == retval)
    {
        /* the value was not found. */
        retval = AGENTD_ERROR_DATASERVICE_NOT_FOUND;
        goto transaction_rollback;
    }
    else if (0 != retval)
    {
        /* some error has occurred. */
        retval = AGENTD_ERROR_DATASERVICE_MDB_GET_FAILURE;
        goto transaction_rollback;
    }

    /* if this value would be truncated, give the caller the real size. */
    if (lval.mv_size > *size)
    {
        retval = AGENTD_ERROR_DATASERVICE_WOULD_TRUNCATE;
        *size = lval.mv_size;
        goto transaction_rollback;
    }

    /* success.  Copy the value to the caller and return. */
    *size = lval.mv_size;
    memcpy(buffer, lval.mv_data, *size);
    retval = AGENTD_STATUS_SUCCESS;

transaction_rollback:
    mdb_txn_abort(txn);

done:
    return retval;
}
