#include <cbmc/model_assert.h>
#include <stdlib.h>

int nondet_status();

#include "shadow_lmdb_internal.h"

int mdb_env_sync(MDB_env* env, int force)
{
    MODEL_ASSERT(PROP_MDB_ENV_OPENED(env));

    return nondet_status();
}
