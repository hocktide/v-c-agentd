#include <cbmc/model_assert.h>
#include <stdlib.h>

int nondet_status();

#include "shadow_lmdb_internal.h"

int mdb_env_open(
    MDB_env* env, const char* path, unsigned int flags, mdb_mode_t mode)
{
    MODEL_ASSERT(PROP_MDB_ENV_CREATED(env));
    MODEL_ASSERT(!PROP_MDB_ENV_OPENED(env));
    MODEL_ASSERT(NULL != path);

    int status = nondet_status();
    if (0 != status)
        return status;

    env->opened = 1;

    return 0;
}
