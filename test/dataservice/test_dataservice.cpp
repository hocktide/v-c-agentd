/**
 * \file test_dataservice.cpp
 *
 * Test the data service private API.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <agentd/inet.h>
#include <gtest/gtest.h>
#include <lmdb.h>
#include <vpr/disposable.h>

#include "../../src/dataservice/dataservice_internal.h"

using namespace std;

/**
 * Test that the data service root context can be initialized.
 */
TEST(dataservice_test, root_context_init)
{
    const char* DB_PATH =
        "build/host/checked/databases/396c499b-ff73-45c5-901f-2e48e2dce4c7";
    char command[1024];
    dataservice_root_context_t ctx;

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, root_context_init_no_permission)
{
    const char* DB_PATH =
        "build/host/checked/databases/c681eefc-d2e0-4111-8638-a64a6a77f216";
    char command[1024];
    dataservice_root_context_t ctx;

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly forbid the capability to create this root context. */
    BITCAP_SET_FALSE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialization should fail. */
    ASSERT_NE(0, dataservice_root_context_init(&ctx, DB_PATH));
}

/**
 * Test that we can reduce the capabilities in the root context -- in this case,
 * we reduce all capabilities except further reducing capabilities, and then we
 * eliminate that capability and demonstrate that it is no longer possible to
 * further reduce capabilities.
 */
TEST(dataservice_test, root_context_reduce_capabilities)
{
    const char* DB_PATH =
        "build/host/checked/databases/c681eefc-d2e0-4111-8638-a64a6a77f216";
    char command[1024];
    dataservice_root_context_t ctx;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly set the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialization should succeed. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, child_context_create)
{
    const char* DB_PATH =
        "build/host/checked/databases/553f6a65-ed63-466d-93d7-193d7b0b8c49";
    char command[1024];
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, child_context_create_denied)
{
    const char* DB_PATH =
        "build/host/checked/databases/553f6a65-ed63-466d-93d7-193d7b0b8c49";
    char command[1024];
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, child_context_close)
{
    const char* DB_PATH =
        "build/host/checked/databases/553f6a65-ed63-466d-93d7-193d7b0b8c49";
    char command[1024];
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, child_context_close_denied)
{
    const char* DB_PATH =
        "build/host/checked/databases/553f6a65-ed63-466d-93d7-193d7b0b8c49";
    char command[1024];
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, global_settings_get)
{
    const char* DB_PATH =
        "build/host/checked/databases/996b0f5d-46b7-4d76-8cfd-fe2433939745";
    char command[1024];
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    char schema_buffer[20];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, global_settings_get_denied)
{
    const char* DB_PATH =
        "build/host/checked/databases/cee8e10d-1ac7-41ed-b33d-524ccda2824e";
    char command[1024];
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    char schema_buffer[20];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, global_settings_get_would_truncate)
{
    const char* DB_PATH =
        "build/host/checked/databases/5a920ef8-14b9-455c-b09a-a2b46e28afc6";
    char command[1024];
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    char schema_buffer[10];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, global_settings_get_not_found)
{
    const char* DB_PATH =
        "build/host/checked/databases/f8ef4552-2124-435f-80e6-746b1ec1ea94";
    char command[1024];
    char schema_buffer[20];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, global_settings_set_get)
{
    const char* DB_PATH =
        "build/host/checked/databases/a1e4c959-0279-4e43-a951-24e81d20c51d";
    char command[1024];
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    char schema_buffer[20];
    size_t schema_buffer_sz = sizeof(schema_buffer);
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, global_settings_set_denied)
{
    const char* DB_PATH =
        "build/host/checked/databases/a1e4c959-0279-4e43-a951-24e81d20c51d";
    char command[1024];
    char SCHEMA_VERSION[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, transaction_get_first_empty)
{
    const char* DB_PATH =
        "build/host/checked/databases/46423abb-fc06-4dd1-9fe6-42f527b3cddb";
    char command[1024];
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, transaction_get_first_empty_with_start_end)
{
    const char* DB_PATH =
        "build/host/checked/databases/523b4370-8723-4fc6-b5d6-ac6e90331cdd";
    char command[1024];
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    MDB_txn* txn;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

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
TEST(dataservice_test, transaction_get_first_no_capability)
{
    const char* DB_PATH =
        "build/host/checked/databases/ff3e5166-5c6b-4e13-a816-c841f3d94274";
    char command[1024];
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, transaction_get_first_happy_path)
{
    uint8_t foo_key[16] = { 0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f };
    uint8_t bar_key[16] = { 0xb5, 0x3e, 0x42, 0x83, 0xc7, 0x76, 0x43, 0x81,
        0xbf, 0x91, 0xdc, 0x88, 0x78, 0x38, 0x2c, 0xe5 };
    const char* DB_PATH =
        "build/host/checked/databases/062ccc38-5205-4e9f-b562-a9530a760a46";
    char command[1024];
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    MDB_txn* txn;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

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
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

    /* insert bar. */
    lkey.mv_size = sizeof(bar->key);
    lkey.mv_data = bar->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(bar_data);
    lval.mv_data = bar;
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

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
TEST(dataservice_test, transaction_get_first_txn_happy_path)
{
    uint8_t foo_key[16] = { 0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f };
    uint8_t bar_key[16] = { 0xb5, 0x3e, 0x42, 0x83, 0xc7, 0x76, 0x43, 0x81,
        0xbf, 0x91, 0xdc, 0x88, 0x78, 0x38, 0x2c, 0xe5 };
    const char* DB_PATH =
        "build/host/checked/databases/a61e3fd1-8c9e-4408-8135-96fdc1f1c85e";
    char command[1024];
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    MDB_txn* txn;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

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
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

    /* insert bar. */
    lkey.mv_size = sizeof(bar->key);
    lkey.mv_data = bar->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(bar_data);
    lval.mv_data = bar;
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

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
TEST(dataservice_test, transaction_get_first_with_node_happy_path)
{
    uint8_t foo_key[16] = { 0x9b, 0xfe, 0xec, 0xc9, 0x28, 0x5d, 0x44, 0xba,
        0x84, 0xdf, 0xd6, 0xfd, 0x3e, 0xe8, 0x79, 0x2f };
    uint8_t bar_key[16] = { 0xb5, 0x3e, 0x42, 0x83, 0xc7, 0x76, 0x43, 0x81,
        0xbf, 0x91, 0xdc, 0x88, 0x78, 0x38, 0x2c, 0xe5 };
    const char* DB_PATH =
        "build/host/checked/databases/062ccc38-5205-4e9f-b562-a9530a760a46";
    char command[1024];
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    MDB_txn* txn;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

    /* insert end. */
    lkey.mv_size = sizeof(end.key);
    lkey.mv_data = end.key;
    lval.mv_size = sizeof(end);
    lval.mv_data = &end;
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

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
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

    /* insert bar. */
    lkey.mv_size = sizeof(bar->key);
    lkey.mv_data = bar->key;
    lval.mv_size = sizeof(data_transaction_node_t) + sizeof(bar_data);
    lval.mv_data = bar;
    ASSERT_EQ(0, mdb_put(txn, details->txn_db, &lkey, &lval, 0));

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
TEST(dataservice_test, transaction_submit_get_first_with_node_happy_path)
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

    const char* DB_PATH =
        "build/host/checked/databases/98f645fb-33e3-4eb1-9a8a-8b88945379e6";
    char command[1024];
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
TEST(dataservice_test, transaction_submit_txn_get_first_with_node_happy_path)
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

    const char* DB_PATH =
        "build/host/checked/databases/05e1f95e-5a55-4d3c-ac03-3efd756972d2";
    char command[1024];
    uint8_t* txn_bytes = NULL;
    size_t txn_size = 0;
    data_transaction_node_t node;
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
 * Test that dataservice_transaction_submit respects the bitcap for this action.
 */
TEST(dataservice_test, transaction_submit_bitcap)
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

    const char* DB_PATH =
        "build/host/checked/databases/98f645fb-33e3-4eb1-9a8a-8b88945379e6";
    char command[1024];
    dataservice_root_context_t ctx;
    dataservice_child_context_t child;
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* create the database directory. */
    snprintf(command, sizeof(command), "mkdir -p %s", DB_PATH);
    system(command);

    /* precondition: ctx is invalid. */
    memset(&ctx, 0xFF, sizeof(ctx));
    /* precondition: disposer is NULL. */
    ctx.hdr.dispose = nullptr;

    /* explicitly grant the capability to create this root context. */
    BITCAP_SET_TRUE(ctx.apicaps, DATASERVICE_API_CAP_LL_ROOT_CONTEXT_CREATE);

    /* initialize the root context given a test data directory. */
    ASSERT_EQ(0, dataservice_root_context_init(&ctx, DB_PATH));

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
    ASSERT_EQ(3,
        dataservice_transaction_submit(
            &child, nullptr, foo_key, foo_artifact, foo_data,
            sizeof(foo_data)));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}
