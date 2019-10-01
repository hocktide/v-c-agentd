#include <cbmc/model_assert.h>
#include <stdlib.h>

#include "shadow_lmdb_internal.h"

void mdb_txn_abort(MDB_txn* txn)
{
    MODEL_ASSERT(PROP_MDB_TXN_BEGUN(txn));

    free(txn);
}
