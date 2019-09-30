#include <cbmc/model_assert.h>
#include <stdlib.h>

int nondet_status();

#include "shadow_lmdb_internal.h"

int mdb_env_create(MDB_env** env)
{
    MODEL_ASSERT(NULL != env);

    int status = nondet_status();
    if (0 != status)
        return status;

    *env = (MDB_env*)malloc(sizeof(MDB_env));
    if (NULL == *env)
        return -1;

    (*env)->created = 1;
    (*env)->opened = 0;
    (*env)->txn_depth = 0;

    return 0;
}
