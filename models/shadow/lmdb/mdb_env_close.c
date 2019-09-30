#include <cbmc/model_assert.h>
#include <stdlib.h>

#include "shadow_lmdb_internal.h"

void mdb_env_close(MDB_env* env)
{
    MODEL_ASSERT(PROP_MDB_ENV_CREATED(env));

    free(env);
}
