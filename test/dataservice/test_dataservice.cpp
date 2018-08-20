/**
 * \file test_dataservice.cpp
 *
 * Test the data service private API.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <gtest/gtest.h>
#include <vpr/disposable.h>

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
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_NEXT_READ));
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
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_NEXT_READ));
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
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_NEXT_READ));
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
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_NEXT_READ));
    EXPECT_FALSE(BITCAP_ISSET(ctx.apicaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE));

    /* the call to reduce capabilities will now fail. */
    ASSERT_NE(0,
        dataservice_root_context_reduce_capabilities(&ctx, reducedcaps));

    /* dispose of the context. */
    dispose((disposable_t*)&ctx);
}
