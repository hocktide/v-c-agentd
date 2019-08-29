#include <cbmc/model_assert.h>
#include <stdlib.h>

#include "shadow_lmdb_internal.h"

void mdb_dbi_close(MDB_env* env, MDB_dbi dbi)
{
    MODEL_ASSERT(PROP_MDB_ENV_OPENED(env));
}
