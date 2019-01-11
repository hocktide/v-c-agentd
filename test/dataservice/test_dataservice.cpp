/**
 * \file test_dataservice.cpp
 *
 * Test the data service private API.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <vccert/certificate_types.h>

#include "test_dataservice.h"

using namespace std;

/**
 * Test that the data service root context can be initialized.
 */
TEST_F(dataservice_test, root_context_init)
{
    dataservice_root_context_t ctx;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* there should be a disposer set. */
    ASSERT_NE(nullptr, ctx.hdr.dispose);

    /* We can't create a root context again. */
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE));

    /* All other capabilities are set by default. */
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_BACKUP));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_RESTORE));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_UPGRADE));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_NEXT_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_PREV_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_WITH_TRANSACTION_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that without the root create capability, we cannot create a root
 * context.
 */
TEST_F(dataservice_test, root_context_init_no_permission)
{
    dataservice_root_context_t ctx;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly forbid the capability to create this root context. */
    BITCAP_SET_FALSE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialization should fail. */
    ASSERT_NE(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));
}

/**
 * Test that we can reduce the capabilities in the root context -- in this case,
 * we reduce all capabilities except further reducing capabilities, and then we
 * eliminate that capability and demonstrate that it is no longer possible to
 * further reduce capabilities.
 */
TEST_F(dataservice_test, root_context_reduce_capabilities)
{
    dataservice_root_context_t ctx;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly set the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialization should succeed. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* We can't create a root context again. */
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE));

    /* All other capabilities are set by default. */
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_BACKUP));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_RESTORE));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_UPGRADE));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_NEXT_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_PREV_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_WITH_TRANSACTION_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP));
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE));

    /* reduce the capabilites to only allow the capabilities to be further
     * reduced. */
    BITCAP_INIT_FALSE(reducedcaps);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS);

    /* the call to reduce capabilities should succeed. */
    ASSERT_EQ(0,
        dataservice_root_context_reduce_capabilities(&ctx, reducedcaps));

    /* We can further reduce capabilities. */
    EXPECT_TRUE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS));

    /* All other capabilities are disabled. */
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_BACKUP));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_RESTORE));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_UPGRADE));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_NEXT_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_PREV_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_WITH_TRANSACTION_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE));

    /* reduce the capabilites to nothing. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* the call to reduce capabilities should succeed. */
    ASSERT_EQ(0,
        dataservice_root_context_reduce_capabilities(&ctx, reducedcaps));

    /* All capabilities are disabled. */
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_BACKUP));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_RESTORE));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_LL_DATABASE_UPGRADE));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_NEXT_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_PREV_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_WITH_TRANSACTION_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE));

    /* the call to reduce capabilities will now fail. */
    ASSERT_NE(0,
        dataservice_root_context_reduce_capabilities(&ctx, reducedcaps));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that a child context can be created from a root context.
 */
TEST_F(dataservice_test, child_context_create)
{
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* there should be a disposer set. */
    ASSERT_NE(nullptr, ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    /* make sure the child create and close contexts are set. */
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* explicitly grant the create and close child caps to the child context. */
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* the child context cannot create other child contexts. */
    EXPECT_FALSE(
        BITCAP_ISSET(
            child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE));

    /* the child context can close itself. */
    EXPECT_TRUE(
        BITCAP_ISSET(
            child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE));

    /* verify that this child context can read transactions. */
    EXPECT_TRUE(
        BITCAP_ISSET(
            child.childcaps, DATASERVICE_API_CAP_APP_TRANSACTION_READ));

    /* verify that other capabilities, like database backup, are disabled. */
    EXPECT_FALSE(
        BITCAP_ISSET(
            child.childcaps, DATASERVICE_API_CAP_LL_DATABASE_BACKUP));

    /* verify that trying to create the child context a second time fails. */
    ASSERT_NE(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that a child context cannot be created from a root context if the root
 * context does not have the create child context capability.
 */
TEST_F(dataservice_test, child_context_create_denied)
{
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* there should be a disposer set. */
    ASSERT_NE(nullptr, ctx.hdr.dispose);

    /* explicitly deny child context creation in the parent context. */
    BITCAP_SET_FALSE(ctx.apicaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    /* make sure the child create and close contexts are set. */
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* explicitly grant the create and close child caps to the child context. */
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* creating a child fails because root cannot create child contexts. */
    ASSERT_NE(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that a child context can be closed.
 */
TEST_F(dataservice_test, child_context_close)
{
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* there should be a disposer set. */
    ASSERT_NE(nullptr, ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    /* make sure the child create and close contexts are set. */
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* explicitly grant the create and close child caps to the child context. */
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* closing the child context succeeds. */
    ASSERT_EQ(0, dataservice_child_context_close(&child));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that closing a child context fails if it lacks the close cap.
 */
TEST_F(dataservice_test, child_context_close_denied)
{
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* there should be a disposer set. */
    ASSERT_NE(nullptr, ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);

    /* make sure the child create context cap is set. */
    BITCAP_SET_TRUE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* explicitly deny child close context cap. */
    BITCAP_SET_FALSE(
        reducedcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* explicitly grant the create and close child caps to the child context. */
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(
        child.childcaps, DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* closing the child context fails. */
    ASSERT_NE(0, dataservice_child_context_close(&child));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that we can query a global setting that is already saved in the
 * database.
 */
TEST_F(dataservice_test, global_settings_get)
{
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    char schema_buffer[20];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* there should be a disposer set. */
    ASSERT_NE(nullptr, ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow global settings queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* hard-set the schema version UUID. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;
    uint64_t key_enum = DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION;
    MDB_txn* txn;
    MDB_val key;
    key.mv_size = sizeof(key_enum);
    key.mv_data = &key_enum;
    MDB_val val;
    val.mv_size = sizeof(SCHEMA_VERSION);
    val.mv_data = SCHEMA_VERSION;
    ASSERT_EQ(0, mdb_txn_begin(details->env, NULL, 0, &txn));
    ASSERT_EQ(0, mdb_put(txn, details->global_db, &key, &val, 0));
    ASSERT_EQ(0, mdb_txn_commit(txn));

    /* precondition: schema data is null. */
    memset(schema_buffer, 0, sizeof(schema_buffer));

    /* querying the global data should succeed. */
    ASSERT_EQ(0,
        dataservice_global_settings_get(
            &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION, schema_buffer,
            &schema_buffer_sz));

    /* the buffer size should be the size of the schema UUID. */
    ASSERT_EQ(sizeof(SCHEMA_VERSION), schema_buffer_sz);

    /* the schema buffer should match the schema UUID. */
    EXPECT_EQ(0, memcmp(schema_buffer, SCHEMA_VERSION, schema_buffer_sz));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that if we are not allowed to query a global setting, the API call
 * fails.
 */
TEST_F(dataservice_test, global_settings_get_denied)
{
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    char schema_buffer[20];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* there should be a disposer set. */
    ASSERT_NE(nullptr, ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    /* don't allow it to query global settings. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* hard-set the schema version UUID. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;
    uint64_t key_enum = DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION;
    MDB_txn* txn;
    MDB_val key;
    key.mv_size = sizeof(key_enum);
    key.mv_data = &key_enum;
    MDB_val val;
    val.mv_size = sizeof(SCHEMA_VERSION);
    val.mv_data = SCHEMA_VERSION;
    ASSERT_EQ(0, mdb_txn_begin(details->env, NULL, 0, &txn));
    ASSERT_EQ(0, mdb_put(txn, details->global_db, &key, &val, 0));
    ASSERT_EQ(0, mdb_txn_commit(txn));

    /* precondition: schema data is null. */
    memset(schema_buffer, 0, sizeof(schema_buffer));

    /* querying the global data should fail. */
    ASSERT_NE(0,
        dataservice_global_settings_get(
            &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION, schema_buffer,
            &schema_buffer_sz));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that we get a truncation error if attempting to query a value with too
 * small of a buffer.
 */
TEST_F(dataservice_test, global_settings_get_would_truncate)
{
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    char schema_buffer[10];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* there should be a disposer set. */
    ASSERT_NE(nullptr, ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow global settings queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* hard-set the schema version UUID. */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;
    uint64_t key_enum = DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION;
    MDB_txn* txn;
    MDB_val key;
    key.mv_size = sizeof(key_enum);
    key.mv_data = &key_enum;
    MDB_val val;
    val.mv_size = sizeof(SCHEMA_VERSION);
    val.mv_data = SCHEMA_VERSION;
    ASSERT_EQ(0, mdb_txn_begin(details->env, NULL, 0, &txn));
    ASSERT_EQ(0, mdb_put(txn, details->global_db, &key, &val, 0));
    ASSERT_EQ(0, mdb_txn_commit(txn));

    /* precondition: schema data is null. */
    memset(schema_buffer, 0, sizeof(schema_buffer));

    /* querying the global data should fail due to truncation. */
    ASSERT_EQ(2,
        dataservice_global_settings_get(
            &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION, schema_buffer,
            &schema_buffer_sz));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that we get a value not found error when querying for a value not in the
 * database.
 */
TEST_F(dataservice_test, global_settings_get_not_found)
{
    char schema_buffer[20];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* there should be a disposer set. */
    ASSERT_NE(nullptr, ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow global settings queries. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* precondition: schema data is null. */
    memset(schema_buffer, 0, sizeof(schema_buffer));

    /* querying the global data should fail due to the value not being faund. */
    ASSERT_EQ(1,
        dataservice_global_settings_get(
            &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION, schema_buffer,
            &schema_buffer_sz));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that we can set a global setting and then get it.
 */
TEST_F(dataservice_test, global_settings_set_get)
{
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    char schema_buffer[20];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* there should be a disposer set. */
    ASSERT_NE(nullptr, ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow global settings put / get. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* setting the global setting should succeed. */
    ASSERT_EQ(0,
        dataservice_global_settings_set(
            &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION, SCHEMA_VERSION,
            sizeof(SCHEMA_VERSION)));

    /* precondition: schema data is null. */
    memset(schema_buffer, 0, sizeof(schema_buffer));

    /* querying the global data should succeed. */
    ASSERT_EQ(0,
        dataservice_global_settings_get(
            &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION, schema_buffer,
            &schema_buffer_sz));

    /* the buffer size should be the size of the schema UUID. */
    ASSERT_EQ(sizeof(SCHEMA_VERSION), schema_buffer_sz);

    /* the schema buffer should match the schema UUID from the set call. */
    EXPECT_EQ(0, memcmp(schema_buffer, SCHEMA_VERSION, schema_buffer_sz));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that global settings set respects the global settings write capability.
 */
TEST_F(dataservice_test, global_settings_set_denied)
{
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* there should be a disposer set. */
    ASSERT_NE(nullptr, ctx.hdr.dispose);

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* setting the global setting should fail. */
    ASSERT_NE(0,
        dataservice_global_settings_set(
            &child, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION, SCHEMA_VERSION,
            sizeof(SCHEMA_VERSION)));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that we transaction_get_first indicates that no transaction is found
 * when the transaction queue is empty.
 */
TEST_F(dataservice_test, transaction_get_first_empty)
{
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* getting the first transaction should return a "not found" result. */
    ASSERT_EQ(1,
        dataservice_transaction_get_first(
            &child, nullptr, nullptr, &txn_bytes, &txn_size));

    /* the transaction buffer should be set to NULL. */
    ASSERT_EQ(nullptr, txn_bytes);

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that we transaction_get_first indicates that no transaction is found
 * when the transaction queue exists and is empty.
 */
TEST_F(dataservice_test, transaction_get_first_empty_with_start_end)
{
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    MDB_txn* txn;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create the start and end transactions. */
    data_transaction_node_t start, end;
    memset(&start, 0, sizeof(start));
    memset(&end, 0, sizeof(end));
    memset(start.key, 0, sizeof(start.key));
    memset(start.prev, 0, sizeof(start.prev));
    memset(start.next, 0xFF, sizeof(start.next));
    memset(end.key, 0xFF, sizeof(end.key));
    memset(end.prev, 0, sizeof(end.key));
    memset(end.next, 0xFF, sizeof(end.key));

    /* get the details */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;

    /* create an insert transaction. */
    ASSERT_EQ(0, mdb_txn_begin(details->env, NULL, 0, &txn));

    /* insert start. */
    MDB_val lkey;
    lkey.mv_size = sizeof(start.key);
    lkey.mv_data = start.key;
    MDB_val lval;
    lval.mv_size = sizeof(start);
    lval.mv_data = &start;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* commit. */
    ASSERT_EQ(0, mdb_txn_commit(txn));

    /* getting the first transaction should return a "not found" result. */
    ASSERT_EQ(1,
        dataservice_transaction_get_first(
            &child, nullptr, nullptr, &txn_bytes, &txn_size));

    /* the transaction buffer should be set to NULL. */
    ASSERT_EQ(nullptr, txn_bytes);

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that we transaction_get_first fails when called without the appropriate
 * capability being set.
 */
TEST_F(dataservice_test, transaction_get_first_no_capability)
{
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* conspicuously, no transaction caps. */

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* getting the first transaction should fail due to missing caps. */
    ASSERT_EQ(3,
        dataservice_transaction_get_first(
            &child, nullptr, nullptr, &txn_bytes, &txn_size));

    /* the transaction buffer should be set to NULL. */
    ASSERT_EQ(nullptr, txn_bytes);

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that we transaction_get_first retrieves the first found transaction.
 */
TEST_F(dataservice_test, transaction_get_first_happy_path)
{
    uint8_t foo_key[16] = { 0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f };
    uint8_t bar_key[16] = { 0xb5, 0x3e, 0x42, 0x83, 0xc7, 0x76, 0x43, 0x81,
        0xbf, 0x91, 0xdc, 0x88, 0x78, 0x38, 0x2c, 0xe5 };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    MDB_txn* txn;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create the start and end transactions. */
    data_transaction_node_t start, end;
    memset(&start, 0, sizeof(start));
    memset(&end, 0, sizeof(end));
    memset(start.key, 0, sizeof(start.key));
    memset(start.prev, 0, sizeof(start.prev));
    memcpy(start.next, foo_key, sizeof(start.next));
    memset(end.key, 0xFF, sizeof(end.key));
    memcpy(end.prev, bar_key, sizeof(end.key));
    memset(end.next, 0xFF, sizeof(end.key));

    /* get the details */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;

    /* create an insert transaction. */
    ASSERT_EQ(0, mdb_txn_begin(details->env, NULL, 0, &txn));

    /* insert start. */
    MDB_val lkey;
    lkey.mv_size = sizeof(start.key);
    lkey.mv_data = start.key;
    MDB_val lval;
    lval.mv_size = sizeof(start);
    lval.mv_data = &start;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* create foo and bar transactions. */
    uint8_t foo_data[5] = { 0xFA, 0x12, 0x22, 0x13, 0x99 };
    uint8_t bar_data[1] = { 0x00 };
    data_transaction_node_t* foo = (data_transaction_node_t*)
        malloc(sizeof(data_transaction_node_t) + sizeof(foo_data));
    data_transaction_node_t* bar = (data_transaction_node_t*)
        malloc(sizeof(data_transaction_node_t) + sizeof(bar_data));
    memset(foo, 0, sizeof(data_transaction_node_t));
    memset(bar, 0, sizeof(data_transaction_node_t));
    memcpy(foo->key, foo_key, sizeof(foo->key));
    memset(foo->prev, 0, sizeof(foo->prev));
    memcpy(foo->next, bar_key, sizeof(foo->next));
    memcpy(((uint8_t*)foo) + sizeof(data_transaction_node_t), foo_data,
        sizeof(foo_data));
    foo->net_txn_cert_size = htonll(sizeof(foo_data));
    memcpy(bar->key, bar_key, sizeof(bar->key));
    memcpy(bar->prev, foo_key, sizeof(bar->prev));
    memset(bar->next, 0xFF, sizeof(bar->next));
    memcpy(((uint8_t*)bar) + sizeof(data_transaction_node_t), bar_data,
        sizeof(bar_data));
    bar->net_txn_cert_size = htonll(sizeof(bar_data));

    /* insert foo. */
    lkey.mv_size = sizeof(foo->key);
    lkey.mv_data = foo->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(foo_data);
    lval.mv_data = foo;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert bar. */
    lkey.mv_size = sizeof(bar->key);
    lkey.mv_data = bar->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(bar_data);
    lval.mv_data = bar;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* commit. */
    ASSERT_EQ(0, mdb_txn_commit(txn));

    /* getting the first transaction should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get_first(
            &child, nullptr, nullptr, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    txn_size = sizeof(foo_data);
    ASSERT_NE(nullptr, txn_bytes);
    ASSERT_EQ(0, memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);

    /* clean up. */
    free(txn_bytes);
    free(foo);
    free(bar);
}

/**
 * Test that we transaction_get_first retrieves the first found transaction
 * while under a transaction.
 */
TEST_F(dataservice_test, transaction_get_first_txn_happy_path)
{
    uint8_t foo_key[16] = { 0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f };
    uint8_t bar_key[16] = { 0xb5, 0x3e, 0x42, 0x83, 0xc7, 0x76, 0x43, 0x81,
        0xbf, 0x91, 0xdc, 0x88, 0x78, 0x38, 0x2c, 0xe5 };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    MDB_txn* txn;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create the start and end transactions. */
    data_transaction_node_t start, end;
    memset(&start, 0, sizeof(start));
    memset(&end, 0, sizeof(end));
    memset(start.key, 0, sizeof(start.key));
    memset(start.prev, 0, sizeof(start.prev));
    memcpy(start.next, foo_key, sizeof(start.next));
    memset(end.key, 0xFF, sizeof(end.key));
    memcpy(end.prev, bar_key, sizeof(end.key));
    memset(end.next, 0xFF, sizeof(end.key));

    /* get the details */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;

    /* create an insert transaction. */
    ASSERT_EQ(0, mdb_txn_begin(details->env, NULL, 0, &txn));

    /* insert start. */
    MDB_val lkey;
    lkey.mv_size = sizeof(start.key);
    lkey.mv_data = start.key;
    MDB_val lval;
    lval.mv_size = sizeof(start);
    lval.mv_data = &start;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* create foo and bar transactions. */
    uint8_t foo_data[5] = { 0xFA, 0x12, 0x22, 0x13, 0x99 };
    uint8_t bar_data[1] = { 0x00 };
    data_transaction_node_t* foo = (data_transaction_node_t*)
        malloc(sizeof(data_transaction_node_t) + sizeof(foo_data));
    data_transaction_node_t* bar = (data_transaction_node_t*)
        malloc(sizeof(data_transaction_node_t) + sizeof(bar_data));
    memset(foo, 0, sizeof(data_transaction_node_t));
    memset(bar, 0, sizeof(data_transaction_node_t));
    memcpy(foo->key, foo_key, sizeof(foo->key));
    memset(foo->prev, 0, sizeof(foo->prev));
    memcpy(foo->next, bar_key, sizeof(foo->next));
    memcpy(((uint8_t*)foo) + sizeof(data_transaction_node_t), foo_data,
        sizeof(foo_data));
    foo->net_txn_cert_size = htonll(sizeof(foo_data));
    memcpy(bar->key, bar_key, sizeof(bar->key));
    memcpy(bar->prev, foo_key, sizeof(bar->prev));
    memset(bar->next, 0xFF, sizeof(bar->next));
    memcpy(((uint8_t*)bar) + sizeof(data_transaction_node_t), bar_data,
        sizeof(bar_data));
    bar->net_txn_cert_size = htonll(sizeof(bar_data));

    /* insert foo. */
    lkey.mv_size = sizeof(foo->key);
    lkey.mv_data = foo->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(foo_data);
    lval.mv_data = foo;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert bar. */
    lkey.mv_size = sizeof(bar->key);
    lkey.mv_data = bar->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(bar_data);
    lval.mv_data = bar;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* commit the transaction. */
    mdb_txn_commit(txn);

    /* create a transaction for use with this call. */
    dataservice_transaction_context_t txn_ctx;
    ASSERT_EQ(0, dataservice_data_txn_begin(&child, &txn_ctx, nullptr, false));

    /* getting the first transaction should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get_first(
            &child, &txn_ctx, nullptr, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    txn_size = sizeof(foo_data);
    ASSERT_NE(nullptr, txn_bytes);
    ASSERT_EQ(0, memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* abort the transaction. */
    dataservice_data_txn_abort(&txn_ctx);

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);

    /* clean up. */
    free(foo);
    free(bar);
}

/**
 * Test that we transaction_get_first retrieves the first found transaction and
 * populates the provided transaction node.
 */
TEST_F(dataservice_test, transaction_get_first_with_node_happy_path)
{
    uint8_t foo_key[16] = { 0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f };
    uint8_t bar_key[16] = { 0xb5, 0x3e, 0x42, 0x83, 0xc7, 0x76, 0x43, 0x81,
        0xbf, 0x91, 0xdc, 0x88, 0x78, 0x38, 0x2c, 0xe5 };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    MDB_txn* txn;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create the start and end transactions. */
    data_transaction_node_t start, end;
    memset(&start, 0, sizeof(start));
    memset(&end, 0, sizeof(end));
    memset(start.key, 0, sizeof(start.key));
    memset(start.prev, 0, sizeof(start.prev));
    memcpy(start.next, foo_key, sizeof(start.next));
    memset(end.key, 0xFF, sizeof(end.key));
    memcpy(end.prev, bar_key, sizeof(end.key));
    memset(end.next, 0xFF, sizeof(end.key));

    /* get the details */
    dataservice_database_details_t* details =
        (dataservice_database_details_t*)ctx.details;

    /* create an insert transaction. */
    ASSERT_EQ(0, mdb_txn_begin(details->env, NULL, 0, &txn));

    /* insert start. */
    MDB_val lkey;
    lkey.mv_size = sizeof(start.key);
    lkey.mv_data = start.key;
    MDB_val lval;
    lval.mv_size = sizeof(start);
    lval.mv_data = &start;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* create foo and bar transactions. */
    uint8_t foo_data[5] = { 0xFA, 0x12, 0x22, 0x13, 0x99 };
    uint8_t bar_data[1] = { 0x00 };
    data_transaction_node_t* foo = (data_transaction_node_t*)
        malloc(sizeof(data_transaction_node_t) + sizeof(foo_data));
    data_transaction_node_t* bar = (data_transaction_node_t*)
        malloc(sizeof(data_transaction_node_t) + sizeof(bar_data));
    memset(foo, 0, sizeof(data_transaction_node_t));
    memset(bar, 0, sizeof(data_transaction_node_t));
    memcpy(foo->key, foo_key, sizeof(foo->key));
    memset(foo->prev, 0, sizeof(foo->prev));
    memcpy(foo->next, bar_key, sizeof(foo->next));
    memcpy(((uint8_t*)foo) + sizeof(data_transaction_node_t), foo_data,
        sizeof(foo_data));
    foo->net_txn_cert_size = htonll(sizeof(foo_data));
    memcpy(bar->key, bar_key, sizeof(bar->key));
    memcpy(bar->prev, foo_key, sizeof(bar->prev));
    memset(bar->next, 0xFF, sizeof(bar->next));
    memcpy(((uint8_t*)bar) + sizeof(data_transaction_node_t), bar_data,
        sizeof(bar_data));
    bar->net_txn_cert_size = htonll(sizeof(bar_data));

    /* insert foo. */
    lkey.mv_size = sizeof(foo->key);
    lkey.mv_data = foo->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(foo_data);
    lval.mv_data = foo;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* insert bar. */
    lkey.mv_size = sizeof(bar->key);
    lkey.mv_data = bar->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(bar_data);
    lval.mv_data = bar;
    ASSERT_EQ(0, mdb_put(txn, details->pq_db, &lkey, &lval, 0));

    /* commit. */
    ASSERT_EQ(0, mdb_txn_commit(txn));

    /* PRECONDITION: node is cleared. */
    memset(&node, 0, sizeof(node));

    /* getting the first transaction should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get_first(
            &child, nullptr, &node, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    txn_size = sizeof(foo_data);
    ASSERT_NE(nullptr, txn_bytes);
    ASSERT_EQ(0, memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* the node should match our expectations for foo_node, allowing us to
     * traverse the transaction queue. */
    uint8_t start_key[16];
    memset(start_key, 0, sizeof(start_key));
    EXPECT_EQ(0, memcmp(node.key, foo_key, sizeof(node.key)));
    EXPECT_EQ(0, memcmp(node.prev, start_key, sizeof(node.prev)));
    EXPECT_EQ(0, memcmp(node.next, bar_key, sizeof(node.next)));
    EXPECT_EQ(txn_size, (size_t)ntohll(node.net_txn_cert_size));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);

    /* clean up. */
    free(txn_bytes);
    free(foo);
    free(bar);
}

/**
 * Test that we can submit a transaction to the transaction queue and retrieve
 * it.
 */
TEST_F(dataservice_test, transaction_submit_get_first_with_node_happy_path)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit and read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submit foo transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo_key, foo_artifact, foo_data,
            sizeof(foo_data)));

    /* PRECONDITION: node is cleared. */
    memset(&node, 0, sizeof(node));

    /* getting the first transaction should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get_first(
            &child, nullptr, &node, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    ASSERT_NE(nullptr, txn_bytes);
    ASSERT_EQ(0, memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* the node should match our expectations for foo_node, allowing us to
     * traverse the transaction queue. */
    uint8_t start_key[16];
    memset(start_key, 0, sizeof(start_key));
    uint8_t end_key[16];
    memset(end_key, 0xFF, sizeof(end_key));
    EXPECT_EQ(0, memcmp(node.key, foo_key, sizeof(node.key)));
    EXPECT_EQ(0, memcmp(node.prev, start_key, sizeof(node.prev)));
    EXPECT_EQ(0, memcmp(node.next, end_key, sizeof(node.next)));
    EXPECT_EQ(sizeof(foo_data), (size_t)ntohll(node.net_txn_cert_size));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);

    /* clean up. */
    free(txn_bytes);
}

/**
 * Test that we can submit a transaction to the transaction queue and retrieve
 * it, while under a transaction.
 */
TEST_F(dataservice_test, transaction_submit_txn_get_first_with_node_happy_path)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit and read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create a transaction for use with this call. */
    dataservice_transaction_context_t txn_ctx;
    ASSERT_EQ(0, dataservice_data_txn_begin(&child, &txn_ctx, nullptr, false));

    /* submit foo transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, &txn_ctx, foo_key, foo_artifact, foo_data,
            sizeof(foo_data)));

    /* PRECONDITION: node is cleared. */
    memset(&node, 0, sizeof(node));

    /* getting the first transaction should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get_first(
            &child, &txn_ctx, &node, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    ASSERT_NE(nullptr, txn_bytes);
    ASSERT_EQ(0, memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* the node should match our expectations for foo_node, allowing us to
     * traverse the transaction queue. */
    uint8_t start_key[16];
    memset(start_key, 0, sizeof(start_key));
    uint8_t end_key[16];
    memset(end_key, 0xFF, sizeof(end_key));
    EXPECT_EQ(0, memcmp(node.key, foo_key, sizeof(node.key)));
    EXPECT_EQ(0, memcmp(node.prev, start_key, sizeof(node.prev)));
    EXPECT_EQ(0, memcmp(node.next, end_key, sizeof(node.next)));
    EXPECT_EQ(sizeof(foo_data), (size_t)ntohll(node.net_txn_cert_size));

    /* abort the transaction. */
    dataservice_data_txn_abort(&txn_ctx);

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that we can submit a transaction to the transaction queue and retrieve
 * it by id.
 */
TEST_F(dataservice_test, transaction_submit_get_with_node_happy_path)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit and read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submit foo transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo_key, foo_artifact, foo_data,
            sizeof(foo_data)));

    /* PRECONDITION: node is cleared. */
    memset(&node, 0, sizeof(node));

    /* getting the transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get(
            &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    ASSERT_NE(nullptr, txn_bytes);
    ASSERT_EQ(0, memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* the node should match our expectations for foo_node, allowing us to
     * traverse the transaction queue. */
    uint8_t start_key[16];
    memset(start_key, 0, sizeof(start_key));
    uint8_t end_key[16];
    memset(end_key, 0xFF, sizeof(end_key));
    EXPECT_EQ(0, memcmp(node.key, foo_key, sizeof(node.key)));
    EXPECT_EQ(0, memcmp(node.prev, start_key, sizeof(node.prev)));
    EXPECT_EQ(0, memcmp(node.next, end_key, sizeof(node.next)));
    EXPECT_EQ(sizeof(foo_data), (size_t)ntohll(node.net_txn_cert_size));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);

    /* clean up. */
    free(txn_bytes);
}

/**
 * Test that we can submit a transaction to the transaction queue and retrieve
 * it by id, while under a transaction.
 */
TEST_F(dataservice_test, transaction_submit_txn_get_with_node_happy_path)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit and read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create a transaction for use with this call. */
    dataservice_transaction_context_t txn_ctx;
    ASSERT_EQ(0, dataservice_data_txn_begin(&child, &txn_ctx, nullptr, false));

    /* submit foo transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, &txn_ctx, foo_key, foo_artifact, foo_data,
            sizeof(foo_data)));

    /* PRECONDITION: node is cleared. */
    memset(&node, 0, sizeof(node));

    /* getting the transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get(
            &child, &txn_ctx, foo_key, &node, &txn_bytes, &txn_size));

    /* the data should match the foo packet exactly. */
    ASSERT_NE(nullptr, txn_bytes);
    ASSERT_EQ(0, memcmp(txn_bytes, foo_data, sizeof(foo_data)));

    /* the node should match our expectations for foo_node, allowing us to
     * traverse the transaction queue. */
    uint8_t start_key[16];
    memset(start_key, 0, sizeof(start_key));
    uint8_t end_key[16];
    memset(end_key, 0xFF, sizeof(end_key));
    EXPECT_EQ(0, memcmp(node.key, foo_key, sizeof(node.key)));
    EXPECT_EQ(0, memcmp(node.prev, start_key, sizeof(node.prev)));
    EXPECT_EQ(0, memcmp(node.next, end_key, sizeof(node.next)));
    EXPECT_EQ(sizeof(foo_data), (size_t)ntohll(node.net_txn_cert_size));

    /* abort the transaction. */
    dataservice_data_txn_abort(&txn_ctx);

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that an ettempt to drop the all zeroes or all FFs transactions results
 * in a "not found" error, even after a transaction has been submitted.
 */
TEST_F(dataservice_test, transaction_drop_00_ff)
{
    uint8_t begin_key[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t end_key[16] = {
        0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff,
        0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff
    };
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit, read, and drop. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* attempt to drop the begin transaction. */
    ASSERT_EQ(1,
        dataservice_transaction_drop(
            &child, nullptr, begin_key));

    /* attempt to drop the end transaction. */
    ASSERT_EQ(1,
        dataservice_transaction_drop(
            &child, nullptr, end_key));

    /* submit foo transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo_key, foo_artifact, foo_data,
            sizeof(foo_data)));

    /* attempt to drop the begin transaction. */
    ASSERT_EQ(1,
        dataservice_transaction_drop(
            &child, nullptr, begin_key));

    /* attempt to drop the end transaction. */
    ASSERT_EQ(1,
        dataservice_transaction_drop(
            &child, nullptr, end_key));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that we can drop an entry in the transaction queue after submitting
 * it.
 */
TEST_F(dataservice_test, transaction_drop)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit, read/first, and drop. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submit foo transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo_key, foo_artifact, foo_data,
            sizeof(foo_data)));

    /* getting the first transaction should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get_first(
            &child, nullptr, &node, &txn_bytes, &txn_size));

    /* this transaction id should be ours. */
    ASSERT_EQ(0, memcmp(node.key, foo_key, 16));

    /* getting the transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get(
            &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* attempt to drop foo transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_drop(
            &child, nullptr, foo_key));

    /* getting the first transaction should fail. */
    ASSERT_EQ(1,
        dataservice_transaction_get_first(
            &child, nullptr, &node, &txn_bytes, &txn_size));

    /* now if we try to get the transaction by id, this fails. */
    ASSERT_EQ(1,
        dataservice_transaction_get(
            &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that other entries are preserved and updated when we drop an entry from
 * the queue.
 */
TEST_F(dataservice_test, transaction_drop_ordering)
{
    uint8_t foo1_key[16] = {
        0x2a, 0x3d, 0xe3, 0x6f, 0x4f, 0x5f, 0x43, 0x75,
        0x8d, 0xaf, 0xb0, 0x74, 0x97, 0x8b, 0x51, 0x67
    };
    uint8_t foo1_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo1_data[16] = {
        0xfa, 0x99, 0xb1, 0x9d, 0x66, 0x7a, 0x4a, 0xe3,
        0x96, 0xf4, 0x50, 0xd6, 0x65, 0xda, 0x11, 0x5c
    };
    uint8_t foo2_key[16] = {
        0xb2, 0xea, 0x70, 0x5c, 0x42, 0xd4, 0x40, 0x21,
        0x96, 0xe1, 0x7e, 0x89, 0xfb, 0x04, 0x9a, 0x33
    };
    uint8_t foo2_artifact[16] = {
        0xeb, 0x18, 0xe9, 0x7b, 0x2e, 0x8a, 0x41, 0xf2,
        0xbf, 0xc5, 0xea, 0x7d, 0x65, 0x2a, 0x71, 0xce
    };
    uint8_t foo2_data[16] = {
        0x83, 0xf3, 0x6a, 0xa4, 0x71, 0xbe, 0x4f, 0xb6,
        0xa0, 0xcf, 0xe5, 0x69, 0x29, 0x23, 0x2b, 0xe0
    };
    uint8_t foo3_key[16] = {
        0x33, 0x48, 0xfd, 0x83, 0xa7, 0xc5, 0x4b, 0xf1,
        0x85, 0x2f, 0x27, 0x99, 0x90, 0x8a, 0xce, 0xbc
    };
    uint8_t foo3_artifact[16] = {
        0xf2, 0x90, 0xce, 0xe0, 0x44, 0x29, 0x49, 0x97,
        0xad, 0x8b, 0xb0, 0x77, 0x06, 0xe2, 0xc1, 0x97
    };
    uint8_t foo3_data[16] = {
        0x4f, 0x61, 0x98, 0x8e, 0x23, 0x84, 0x49, 0x29,
        0x92, 0x76, 0x84, 0x06, 0x42, 0x36, 0x3a, 0x6b
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit, read/first, and drop. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submit foo1 transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo1_key, foo1_artifact, foo1_data,
            sizeof(foo1_data)));

    /* submit foo2 transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo2_key, foo2_artifact, foo2_data,
            sizeof(foo2_data)));

    /* submit foo3 transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo3_key, foo3_artifact, foo3_data,
            sizeof(foo3_data)));

    /* getting the first transaction should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get_first(
            &child, nullptr, &node, &txn_bytes, &txn_size));

    /* this should match foo1. */
    uint8_t begin_key[16];
    memset(begin_key, 0, sizeof(begin_key));
    uint8_t end_key[16];
    memset(end_key, 0xff, sizeof(end_key));
    ASSERT_EQ(0, memcmp(node.key, foo1_key, 16));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo1_artifact, 16));
    ASSERT_EQ(0, memcmp(node.prev, begin_key, 16));
    ASSERT_EQ(0, memcmp(node.next, foo2_key, 16));
    ASSERT_EQ(sizeof(foo1_data), txn_size);
    ASSERT_EQ(0, memcmp(txn_bytes, foo1_data, sizeof(foo1_data)));

    /* getting the transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get(
            &child, nullptr, foo1_key, &node, &txn_bytes, &txn_size));

    /* this should match foo1. */
    ASSERT_EQ(0, memcmp(node.key, foo1_key, 16));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo1_artifact, 16));
    ASSERT_EQ(0, memcmp(node.prev, begin_key, 16));
    ASSERT_EQ(0, memcmp(node.next, foo2_key, 16));
    ASSERT_EQ(sizeof(foo1_data), txn_size);
    ASSERT_EQ(0, memcmp(txn_bytes, foo1_data, sizeof(foo1_data)));

    /* getting the next transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get(
            &child, nullptr, node.next, &node, &txn_bytes, &txn_size));

    /* this should match foo2. */
    ASSERT_EQ(0, memcmp(node.key, foo2_key, 16));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo2_artifact, 16));
    ASSERT_EQ(0, memcmp(node.prev, foo1_key, 16));
    ASSERT_EQ(0, memcmp(node.next, foo3_key, 16));
    ASSERT_EQ(sizeof(foo2_data), txn_size);
    ASSERT_EQ(0, memcmp(txn_bytes, foo2_data, sizeof(foo2_data)));

    /* getting the next transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get(
            &child, nullptr, node.next, &node, &txn_bytes, &txn_size));

    /* this should match foo3. */
    ASSERT_EQ(0, memcmp(node.key, foo3_key, 16));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo3_artifact, 16));
    ASSERT_EQ(0, memcmp(node.prev, foo2_key, 16));
    ASSERT_EQ(0, memcmp(node.next, end_key, 16));
    ASSERT_EQ(sizeof(foo3_data), txn_size);
    ASSERT_EQ(0, memcmp(txn_bytes, foo3_data, sizeof(foo3_data)));

    /* attempt to drop foo2 transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_drop(
            &child, nullptr, foo2_key));

    /* now if we try to get the transaction by id, this fails. */
    ASSERT_EQ(1,
        dataservice_transaction_get(
            &child, nullptr, foo2_key, &node, &txn_bytes, &txn_size));

    /* getting the first transaction should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get_first(
            &child, nullptr, &node, &txn_bytes, &txn_size));

    /* this should match foo1. */
    ASSERT_EQ(0, memcmp(node.key, foo1_key, 16));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo1_artifact, 16));
    ASSERT_EQ(0, memcmp(node.prev, begin_key, 16));
    ASSERT_EQ(0, memcmp(node.next, foo3_key, 16));
    ASSERT_EQ(sizeof(foo1_data), txn_size);
    ASSERT_EQ(0, memcmp(txn_bytes, foo1_data, sizeof(foo1_data)));

    /* getting the next transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get(
            &child, nullptr, node.next, &node, &txn_bytes, &txn_size));

    /* this should match foo3. */
    ASSERT_EQ(0, memcmp(node.key, foo3_key, 16));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo3_artifact, 16));
    ASSERT_EQ(0, memcmp(node.prev, foo1_key, 16));
    ASSERT_EQ(0, memcmp(node.next, end_key, 16));
    ASSERT_EQ(sizeof(foo3_data), txn_size);
    ASSERT_EQ(0, memcmp(txn_bytes, foo3_data, sizeof(foo3_data)));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that other entries are preserved and updated when we drop the first
 * entry from the queue.
 */
TEST_F(dataservice_test, transaction_drop_first_ordering)
{
    uint8_t foo1_key[16] = {
        0x2a, 0x3d, 0xe3, 0x6f, 0x4f, 0x5f, 0x43, 0x75,
        0x8d, 0xaf, 0xb0, 0x74, 0x97, 0x8b, 0x51, 0x67
    };
    uint8_t foo1_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo1_data[16] = {
        0xfa, 0x99, 0xb1, 0x9d, 0x66, 0x7a, 0x4a, 0xe3,
        0x96, 0xf4, 0x50, 0xd6, 0x65, 0xda, 0x11, 0x5c
    };
    uint8_t foo2_key[16] = {
        0xb2, 0xea, 0x70, 0x5c, 0x42, 0xd4, 0x40, 0x21,
        0x96, 0xe1, 0x7e, 0x89, 0xfb, 0x04, 0x9a, 0x33
    };
    uint8_t foo2_artifact[16] = {
        0xeb, 0x18, 0xe9, 0x7b, 0x2e, 0x8a, 0x41, 0xf2,
        0xbf, 0xc5, 0xea, 0x7d, 0x65, 0x2a, 0x71, 0xce
    };
    uint8_t foo2_data[16] = {
        0x83, 0xf3, 0x6a, 0xa4, 0x71, 0xbe, 0x4f, 0xb6,
        0xa0, 0xcf, 0xe5, 0x69, 0x29, 0x23, 0x2b, 0xe0
    };
    uint8_t foo3_key[16] = {
        0x33, 0x48, 0xfd, 0x83, 0xa7, 0xc5, 0x4b, 0xf1,
        0x85, 0x2f, 0x27, 0x99, 0x90, 0x8a, 0xce, 0xbc
    };
    uint8_t foo3_artifact[16] = {
        0xf2, 0x90, 0xce, 0xe0, 0x44, 0x29, 0x49, 0x97,
        0xad, 0x8b, 0xb0, 0x77, 0x06, 0xe2, 0xc1, 0x97
    };
    uint8_t foo3_data[16] = {
        0x4f, 0x61, 0x98, 0x8e, 0x23, 0x84, 0x49, 0x29,
        0x92, 0x76, 0x84, 0x06, 0x42, 0x36, 0x3a, 0x6b
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction submit, read/first, and drop. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submit foo1 transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo1_key, foo1_artifact, foo1_data,
            sizeof(foo1_data)));

    /* submit foo2 transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo2_key, foo2_artifact, foo2_data,
            sizeof(foo2_data)));

    /* submit foo3 transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo3_key, foo3_artifact, foo3_data,
            sizeof(foo3_data)));

    /* getting the first transaction should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get_first(
            &child, nullptr, &node, &txn_bytes, &txn_size));

    /* this should match foo1. */
    uint8_t begin_key[16];
    memset(begin_key, 0, sizeof(begin_key));
    uint8_t end_key[16];
    memset(end_key, 0xff, sizeof(end_key));
    ASSERT_EQ(0, memcmp(node.key, foo1_key, 16));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo1_artifact, 16));
    ASSERT_EQ(0, memcmp(node.prev, begin_key, 16));
    ASSERT_EQ(0, memcmp(node.next, foo2_key, 16));
    ASSERT_EQ(sizeof(foo1_data), txn_size);
    ASSERT_EQ(0, memcmp(txn_bytes, foo1_data, sizeof(foo1_data)));

    /* getting the transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get(
            &child, nullptr, foo1_key, &node, &txn_bytes, &txn_size));

    /* this should match foo1. */
    ASSERT_EQ(0, memcmp(node.key, foo1_key, 16));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo1_artifact, 16));
    ASSERT_EQ(0, memcmp(node.prev, begin_key, 16));
    ASSERT_EQ(0, memcmp(node.next, foo2_key, 16));
    ASSERT_EQ(sizeof(foo1_data), txn_size);
    ASSERT_EQ(0, memcmp(txn_bytes, foo1_data, sizeof(foo1_data)));

    /* getting the next transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get(
            &child, nullptr, node.next, &node, &txn_bytes, &txn_size));

    /* this should match foo2. */
    ASSERT_EQ(0, memcmp(node.key, foo2_key, 16));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo2_artifact, 16));
    ASSERT_EQ(0, memcmp(node.prev, foo1_key, 16));
    ASSERT_EQ(0, memcmp(node.next, foo3_key, 16));
    ASSERT_EQ(sizeof(foo2_data), txn_size);
    ASSERT_EQ(0, memcmp(txn_bytes, foo2_data, sizeof(foo2_data)));

    /* getting the next transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get(
            &child, nullptr, node.next, &node, &txn_bytes, &txn_size));

    /* this should match foo3. */
    ASSERT_EQ(0, memcmp(node.key, foo3_key, 16));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo3_artifact, 16));
    ASSERT_EQ(0, memcmp(node.prev, foo2_key, 16));
    ASSERT_EQ(0, memcmp(node.next, end_key, 16));
    ASSERT_EQ(sizeof(foo3_data), txn_size);
    ASSERT_EQ(0, memcmp(txn_bytes, foo3_data, sizeof(foo3_data)));

    /* attempt to drop foo1 transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_drop(
            &child, nullptr, foo1_key));

    /* now if we try to get the transaction by id, this fails. */
    ASSERT_EQ(1,
        dataservice_transaction_get(
            &child, nullptr, foo1_key, &node, &txn_bytes, &txn_size));

    /* getting the first transaction should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get_first(
            &child, nullptr, &node, &txn_bytes, &txn_size));

    /* this should match foo2. */
    ASSERT_EQ(0, memcmp(node.key, foo2_key, 16));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo2_artifact, 16));
    ASSERT_EQ(0, memcmp(node.prev, begin_key, 16));
    ASSERT_EQ(0, memcmp(node.next, foo3_key, 16));
    ASSERT_EQ(sizeof(foo2_data), txn_size);
    ASSERT_EQ(0, memcmp(txn_bytes, foo2_data, sizeof(foo2_data)));

    /* getting the next transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get(
            &child, nullptr, node.next, &node, &txn_bytes, &txn_size));

    /* this should match foo3. */
    ASSERT_EQ(0, memcmp(node.key, foo3_key, 16));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo3_artifact, 16));
    ASSERT_EQ(0, memcmp(node.prev, foo2_key, 16));
    ASSERT_EQ(0, memcmp(node.next, end_key, 16));
    ASSERT_EQ(sizeof(foo3_data), txn_size);
    ASSERT_EQ(0, memcmp(txn_bytes, foo3_data, sizeof(foo3_data)));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that dataservice_transaction_submit respects the bitcap for this action.
 */
TEST_F(dataservice_test, transaction_submit_bitcap)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_artifact[16] = {
        0xcf, 0xa1, 0x51, 0xc4, 0x7c, 0x0f, 0x4d, 0xbd,
        0xa0, 0xd6, 0x22, 0x51, 0x34, 0xd1, 0x61, 0xdc
    };
    uint8_t foo_data[5] = {
        0Xfa, 0X12, 0X22, 0X13, 0X99
    };
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* only allow transaction read. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* submitting foo transaction fails. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_NOT_AUTHORIZED,
        dataservice_transaction_submit(
            &child, nullptr, foo_key, foo_artifact, foo_data,
            sizeof(foo_data)));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that dataservice_transaction_get_first respects the bitcap for this
 * action.
 */
TEST_F(dataservice_test, transaction_get_first_bitcap)
{
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* getting the first transaction fails due no capabilities. */
    data_transaction_node_t node;
    ASSERT_EQ(3,
        dataservice_transaction_get_first(
            &child, nullptr, &node, &txn_bytes, &txn_size));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that dataservice_transaction_get respects the bitcap for this action.
 */
TEST_F(dataservice_test, transaction_get_bitcap)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* getting the first transaction fails due no capabilities. */
    data_transaction_node_t node;
    ASSERT_EQ(3,
        dataservice_transaction_get(
            &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that dataservice_transaction_drop respects the bitcap for this action.
 */
TEST_F(dataservice_test, transaction_drop_bitcap)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* dropping a transaction fails due to no capability. */
    ASSERT_EQ(3,
        dataservice_transaction_drop(
            &child, nullptr, foo_key));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}

/**
 * Test that we can add a transaction to the transaction queue, create a block
 * containing this transaction, and the dataservice_block_make API call
 * automatically drops this transaction.
 */
TEST_F(dataservice_test, transaction_make_block_simple)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_prev[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t foo_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo_block_id[16] = {
        0x96, 0x1e, 0xdd, 0x16, 0xbd, 0xa6, 0x4b, 0x9d,
        0x93, 0xac, 0x40, 0xd4, 0x74, 0x85, 0x0d, 0xe5
    };
    uint8_t* foo_cert = nullptr;
    size_t foo_cert_length = 0;
    uint8_t* foo_block_cert = nullptr;
    size_t foo_block_cert_length = 0;
    string DB_PATH;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    data_transaction_node_t node;
    data_artifact_record_t foo_artifact_record;
    data_block_node_t block_node;
    uint8_t* txn_bytes;
    size_t txn_size;
    uint8_t* block_txn_bytes;
    size_t block_txn_size;
    uint8_t block_id_for_height_1[16];

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_ARTIFACT_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_BY_HEIGHT_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* verify that our block does not exist. */
    ASSERT_EQ(1,
        dataservice_block_get(
            &child, nullptr, foo_block_id, &block_node,
            &block_txn_bytes, &block_txn_size));

    /* verify that a block ID does not exist for block height 1. */
    ASSERT_EQ(1,
        dataservice_block_id_by_height_get(
            &child, nullptr, 1, block_id_for_height_1));

    /* verify that our artifact does not exist. */
    /* getting the artifact record by artifact id should return not found. */
    ASSERT_EQ(1,
        dataservice_artifact_get(
            &child, nullptr, foo_artifact, &foo_artifact_record));

    /* create foo transaction. */
    ASSERT_EQ(0,
        create_dummy_transaction(
            foo_key, foo_prev, foo_artifact, &foo_cert, &foo_cert_length));

    /* submit foo transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo_key, foo_artifact, foo_cert,
            foo_cert_length));

    /* getting the transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_transaction_get(
            &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));
    free(txn_bytes);

    /* create foo block. */
    ASSERT_EQ(0,
        create_dummy_block(
            &builder_opts,
            foo_block_id, vccert_certificate_type_uuid_root_block, 1,
            &foo_block_cert, &foo_block_cert_length,
            foo_cert, foo_cert_length,
            nullptr));

    /* getting the block transaction by id should return not found. */
    ASSERT_EQ(1,
        dataservice_block_transaction_get(
            &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* make block. */
    ASSERT_EQ(0,
        dataservice_block_make(
            &child, nullptr, foo_block_id,
            foo_block_cert, foo_block_cert_length));

    /* getting the transaction by id should return not found. */
    ASSERT_EQ(1,
        dataservice_transaction_get(
            &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));

    /* getting the block transaction by id should return success. */
    ASSERT_EQ(0,
        dataservice_block_transaction_get(
            &child, nullptr, foo_key, &node, &txn_bytes, &txn_size));
    free(txn_bytes);

    /* getting the block record by block id should return success. */
    ASSERT_EQ(0,
        dataservice_block_get(
            &child, nullptr, foo_block_id, &block_node,
            &block_txn_bytes, &block_txn_size));
    /* the key should match our block id. */
    ASSERT_EQ(0, memcmp(block_node.key, foo_block_id, 16));
    ASSERT_EQ(0, memcmp(block_node.first_transaction_id, foo_key, 16));
    ASSERT_EQ(1U, ntohll(block_node.net_block_height));

    /* verify that a block ID exists for block height 1. */
    ASSERT_EQ(0,
        dataservice_block_id_by_height_get(
            &child, nullptr, 1, block_id_for_height_1));
    /* this block ID matches our block ID. */
    EXPECT_EQ(0, memcmp(foo_block_id, block_id_for_height_1, 16));

    /* getting the artifact record by artifact id should return success. */
    ASSERT_EQ(0,
        dataservice_artifact_get(
            &child, nullptr, foo_artifact, &foo_artifact_record));
    /* the key should match the artifact ID. */
    ASSERT_EQ(0, memcmp(foo_artifact_record.key, foo_artifact, 16));
    /* the first transaction should be the foo transaction. */
    ASSERT_EQ(0, memcmp(foo_artifact_record.txn_first, foo_key, 16));
    /* the latest transaction should be the foo transaction. */
    ASSERT_EQ(0, memcmp(foo_artifact_record.txn_latest, foo_key, 16));
    /* the first height for this artifact should be 1. */
    ASSERT_EQ(1U, ntohll(foo_artifact_record.net_height_first));
    /* the latest height for this artifact should be 1. */
    ASSERT_EQ(1U, ntohll(foo_artifact_record.net_height_latest));

    /* clean up. */
    dispose((disposable_t*)&ctx);
    free(foo_cert);
    free(foo_block_cert);
}

/**
 * Test that the bitset is enforced for making blocks.
 */
TEST_F(dataservice_test, transaction_make_block_bitset)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_prev[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t foo_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo_block_id[16] = {
        0x96, 0x1e, 0xdd, 0x16, 0xbd, 0xa6, 0x4b, 0x9d,
        0x93, 0xac, 0x40, 0xd4, 0x74, 0x85, 0x0d, 0xe5
    };
    uint8_t* foo_cert = nullptr;
    size_t foo_cert_length = 0;
    uint8_t* foo_block_cert = nullptr;
    size_t foo_block_cert_length = 0;
    string DB_PATH;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    /* DO NOT ALLOW BLOCK_WRITE. */
    /*BITCAP_SET_TRUE(reducedcaps,
                    DATASERVICE_API_CAP_APP_BLOCK_WRITE);*/
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create foo transaction. */
    ASSERT_EQ(0,
        create_dummy_transaction(
            foo_key, foo_prev, foo_artifact, &foo_cert, &foo_cert_length));

    /* submit foo transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo_key, foo_artifact, foo_cert,
            foo_cert_length));

    /* create foo block. */
    ASSERT_EQ(0,
        create_dummy_block(
            &builder_opts,
            foo_block_id, vccert_certificate_type_uuid_root_block, 1,
            &foo_block_cert, &foo_block_cert_length,
            foo_cert, foo_cert_length,
            nullptr));

    /* make block should fail because of missing capability. */
    ASSERT_EQ(3,
        dataservice_block_make(
            &child, nullptr, foo_block_id,
            foo_block_cert, foo_block_cert_length));

    /* clean up. */
    dispose((disposable_t*)&ctx);
    free(foo_cert);
    free(foo_block_cert);
}

/**
 * Test that appending a block with an invalid height will fail.
 */
TEST_F(dataservice_test, transaction_make_block_bad_height)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_prev[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t foo_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo_block_id[16] = {
        0x96, 0x1e, 0xdd, 0x16, 0xbd, 0xa6, 0x4b, 0x9d,
        0x93, 0xac, 0x40, 0xd4, 0x74, 0x85, 0x0d, 0xe5
    };
    uint8_t* foo_cert = nullptr;
    size_t foo_cert_length = 0;
    uint8_t* foo_block_cert = nullptr;
    size_t foo_block_cert_length = 0;
    string DB_PATH;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create foo transaction. */
    ASSERT_EQ(0,
        create_dummy_transaction(
            foo_key, foo_prev, foo_artifact, &foo_cert, &foo_cert_length));

    /* submit foo transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo_key, foo_artifact, foo_cert,
            foo_cert_length));

    /* create foo block with invalid 0 block height. */
    ASSERT_EQ(0,
        create_dummy_block(
            &builder_opts,
            foo_block_id, vccert_certificate_type_uuid_root_block, 0,
            &foo_block_cert, &foo_block_cert_length,
            foo_cert, foo_cert_length,
            nullptr));

    /* make block fails due to invalid block height. */
    ASSERT_EQ(9,
        dataservice_block_make(
            &child, nullptr, foo_block_id,
            foo_block_cert, foo_block_cert_length));

    /* clean up. */
    dispose((disposable_t*)&ctx);
    free(foo_cert);
    free(foo_block_cert);
}

/**
 * Test that appending a block with an invalid previous block ID will fail.
 */
TEST_F(dataservice_test, transaction_make_block_bad_prev_block_id)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_prev[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t foo_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo_block_id[16] = {
        0x96, 0x1e, 0xdd, 0x16, 0xbd, 0xa6, 0x4b, 0x9d,
        0x93, 0xac, 0x40, 0xd4, 0x74, 0x85, 0x0d, 0xe5
    };
    uint8_t* foo_cert = nullptr;
    size_t foo_cert_length = 0;
    uint8_t* foo_block_cert = nullptr;
    size_t foo_block_cert_length = 0;
    string DB_PATH;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create foo transaction. */
    ASSERT_EQ(0,
        create_dummy_transaction(
            foo_key, foo_prev, foo_artifact, &foo_cert, &foo_cert_length));

    /* submit foo transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo_key, foo_artifact, foo_cert,
            foo_cert_length));

    /* create foo block with invalid previous block ID. */
    ASSERT_EQ(0,
        create_dummy_block(
            &builder_opts,
            foo_block_id, zero_uuid, 1,
            &foo_block_cert, &foo_block_cert_length,
            foo_cert, foo_cert_length,
            nullptr));

    /* make block fails due to invalid previous block ID. */
    ASSERT_EQ(10,
        dataservice_block_make(
            &child, nullptr, foo_block_id,
            foo_block_cert, foo_block_cert_length));

    /* clean up. */
    dispose((disposable_t*)&ctx);
    free(foo_cert);
    free(foo_block_cert);
}

/**
 * Test that appending a block with an invalid block ID will fail.
 */
TEST_F(dataservice_test, transaction_make_block_bad_block_id)
{
    uint8_t foo_key[16] = {
        0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f
    };
    uint8_t foo_prev[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t foo_artifact[16] = {
        0xef, 0x44, 0xe7, 0xb4, 0xbf, 0x39, 0x45, 0xe4,
        0xb3, 0x4b, 0x6e, 0x82, 0xee, 0x41, 0x76, 0x21
    };
    uint8_t foo_block_id[16] = {
        0x96, 0x1e, 0xdd, 0x16, 0xbd, 0xa6, 0x4b, 0x9d,
        0x93, 0xac, 0x40, 0xd4, 0x74, 0x85, 0x0d, 0xe5
    };
    uint8_t* foo_cert = nullptr;
    size_t foo_cert_length = 0;
    uint8_t* foo_block_cert = nullptr;
    size_t foo_block_cert_length = 0;
    string DB_PATH;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH.c_str()));

    /* create a reduced capabilities set for the child context. */
    BITCAP_INIT_FALSE(reducedcaps);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);

    /* explicitly grant the capability to create child contexts in the child
     * context. */
    BITCAP_SET_TRUE(child.childcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);

    /* create a child context using this reduced capabilities set. */
    ASSERT_EQ(0, dataservice_child_context_create(&ctx, &child, reducedcaps));

    /* create foo transaction. */
    ASSERT_EQ(0,
        create_dummy_transaction(
            foo_key, foo_prev, foo_artifact, &foo_cert, &foo_cert_length));

    /* submit foo transaction. */
    ASSERT_EQ(0,
        dataservice_transaction_submit(
            &child, nullptr, foo_key, foo_artifact, foo_cert,
            foo_cert_length));

    /* create foo block with invalid block ID (root block ID). */
    ASSERT_EQ(0,
        create_dummy_block(
            &builder_opts,
            vccert_certificate_type_uuid_root_block,
            vccert_certificate_type_uuid_root_block, 1,
            &foo_block_cert, &foo_block_cert_length,
            foo_cert, foo_cert_length,
            nullptr));

    /* make block fails due to invalid block ID. */
    ASSERT_EQ(11,
        dataservice_block_make(
            &child, nullptr, foo_block_id,
            foo_block_cert, foo_block_cert_length));

    /* clean up. */
    dispose((disposable_t*)&ctx);
    free(foo_cert);
    free(foo_block_cert);
}
