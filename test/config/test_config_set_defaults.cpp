/**
 * \file test_config_set_defaults.cpp
 *
 * Test that we can set reasonable defaults for config data.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <gtest/gtest.h>
#include <agentd/config.h>
#include <iostream>
#include <vector>
#include <string>
#include <vpr/disposable.h>

extern "C" {
#include <config/agentd.tab.h>
#include <config/agentd.yy.h>
}

using namespace std;

/**
 * \brief Simple user context structure for testing.
 */
struct test_context
{
    disposable_t hdr;
    vector<string> errors;
    agent_config_t* config;
};

/**
 * \brief Dispose a test context.
 */
static void test_context_dispose(void* disp)
{
    test_context* ctx = (test_context*)disp;

    if (nullptr != ctx->config)
        dispose((disposable_t*)ctx->config);
}

/**
 * \brief Initialize a test_context structure.
 */
static void test_context_init(test_context* ctx)
{
    ctx->hdr.dispose = &test_context_dispose;
    ctx->config = nullptr;
}

/**
 * \brief Simple error setting override.
 */
static void set_error(config_context_t* context, const char* msg)
{
    test_context* ctx = (test_context*)context->user_context;

    ctx->errors.push_back(msg);
}

/**
 * \brief Simple value setting override.
 */
static void config_callback(config_context_t* context, agent_config_t* config)
{
    test_context* ctx = (test_context*)context->user_context;

    ctx->config = config;
}

/**
 * Test that all defaults are set.
 */
TEST(config_set_defaults_test, empty_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    bootstrap_config_t bconf;

    /* set up parse of empty config. */
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
    ASSERT_EQ(0U, user_context.errors.size());

    /* initialize bootstrap config. */
    bootstrap_config_init(&bconf);
    bconf.prefix_dir = strdup("build/isolation");
    system("mkdir -p build/isolation");

    /* PRECONDITIONS: all config values are unset. */
    ASSERT_EQ(nullptr, user_context.config->logdir);
    ASSERT_FALSE(user_context.config->loglevel_set);
    ASSERT_FALSE(user_context.config->block_max_seconds_set);
    ASSERT_FALSE(user_context.config->block_max_transactions_set);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* set the defaults for this config. */
    ASSERT_EQ(0, config_set_defaults(user_context.config, &bconf));

    /* POSTCONDITIONS: all config values have their defaults. */
    ASSERT_STREQ("/log", user_context.config->logdir);
    ASSERT_TRUE(user_context.config->loglevel_set);
    ASSERT_EQ(4, user_context.config->loglevel);
    ASSERT_TRUE(user_context.config->block_max_seconds_set);
    ASSERT_EQ(5, user_context.config->block_max_seconds);
    ASSERT_TRUE(user_context.config->block_max_transactions_set);
    ASSERT_EQ(500, user_context.config->block_max_transactions);
    ASSERT_STREQ("/root/secret.cert", user_context.config->secret);
    ASSERT_STREQ("/root/root.cert", user_context.config->rootblock);
    ASSERT_STREQ("/data", user_context.config->datastore);
    ASSERT_NE(nullptr, user_context.config->listen_head);
    ASSERT_STREQ(bconf.prefix_dir, user_context.config->chroot);
    ASSERT_NE(nullptr, user_context.config->usergroup);
    ASSERT_STREQ("veloagent", user_context.config->usergroup->user);
    ASSERT_STREQ("veloagent", user_context.config->usergroup->group);

    /* clean up. */
    dispose((disposable_t*)&user_context);
    dispose((disposable_t*)&bconf);
}
