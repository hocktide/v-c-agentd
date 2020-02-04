#include <cbmc/model_assert.h>
#include <stdlib.h>

int nondet_status();

#include "shadow_lmdb_internal.h"

int mdb_env_set_maxdbs(MDB_env* env, MDB_dbi dbs)
{
    MODEL_ASSERT(PROP_MDB_ENV_CREATED(env));

    return nondet_status();
}
