#include <cbmc/model_assert.h>
#include <stdlib.h>

int nondet_status();

#include "shadow_lmdb_internal.h"

int mdb_txn_commit(MDB_txn* txn)
{
    MODEL_ASSERT(PROP_MDB_TXN_BEGUN(txn));

    free(txn);

    return nondet_status();
}
