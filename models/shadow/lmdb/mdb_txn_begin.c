#include <cbmc/model_assert.h>
#include <stdlib.h>

int nondet_status();

#include "shadow_lmdb_internal.h"

int mdb_txn_begin(
    MDB_env* env, MDB_txn* parent, unsigned int flags, MDB_txn** txn)
{
    MODEL_ASSERT(PROP_MDB_ENV_CREATED(env));
    MODEL_ASSERT(PROP_MDB_ENV_OPENED(env));
    MODEL_ASSERT(NULL != txn);

    int status = nondet_status();
    if (0 != status)
        return status;

    *txn = (MDB_txn*)malloc(sizeof(MDB_txn));
    if (NULL == *txn)
        return -1;

    (*txn)->in_txn = 1;
    (*txn)->parent = parent;

    return 0;
}
