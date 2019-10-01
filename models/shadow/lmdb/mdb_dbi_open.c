#include <cbmc/model_assert.h>
#include <stdlib.h>

int nondet_status();
int nondet_dbi();

#include "shadow_lmdb_internal.h"

int mdb_dbi_open(
    MDB_txn* txn, const char* name, unsigned int flags, MDB_dbi* dbi)
{
    MODEL_ASSERT(PROP_MDB_TXN_BEGUN(txn));
    MODEL_ASSERT(NULL != dbi);

    *dbi = nondet_dbi();

    return nondet_status();
}
