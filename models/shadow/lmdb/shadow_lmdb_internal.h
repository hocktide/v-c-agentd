#pragma once

#include <lmdb.h>

struct MDB_env
{
    int created;
    int opened;
    int txn_depth;
};

struct MDB_txn
{
    int in_txn;
    MDB_txn* parent;
};

#define PROP_MDB_ENV_CREATED(env) \
    (NULL != (env) && (env)->created == 1)

#define PROP_MDB_ENV_OPENED(env) \
    (PROP_MDB_ENV_CREATED(env) && (env)->opened == 1)

#define PROP_MDB_TXN_BEGUN(txn) \
    (NULL != (txn) && (txn)->in_txn == 1)
