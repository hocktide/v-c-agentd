#include <cbmc/model_assert.h>
#include <stdlib.h>

int nondet_status();

#include "shadow_lmdb_internal.h"

int mdb_env_set_mapsize(MDB_env* env, size_t size)
{
    MODEL_ASSERT(PROP_MDB_ENV_CREATED(env));

    return nondet_status();
}
