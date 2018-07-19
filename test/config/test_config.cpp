/**
 * \file test_config.cpp
 *
 * Test the config parser.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <gtest/gtest.h>
#include <agentd/config.h>
#include <iostream>
#include <vector>
#include <string>

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
    vector<string> errors;
    agent_config_t* config;
};

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
 * Test that an empty config file produces a blank config.
 */
TEST(config_test, empty_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    user_context.config = nullptr;

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    ASSERT_EQ(0U, user_context.errors.size());

    /* verify user config. */
    ASSERT_NE(nullptr, user_context.config);
    ASSERT_EQ(nullptr, user_context.config->logdir);
    ASSERT_FALSE(user_context.config->loglevel_set);
    ASSERT_EQ(0L, user_context.config->loglevel);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
}

/**
 * Test that a logdir setting adds this data to the config.
 */
TEST(config_test, logdir_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    user_context.config = nullptr;

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("logdir log", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    ASSERT_EQ(0U, user_context.errors.size());

    /* verify user config. */
    ASSERT_NE(nullptr, user_context.config);
    ASSERT_STREQ("log", user_context.config->logdir);
    ASSERT_FALSE(user_context.config->loglevel_set);
    ASSERT_EQ(0L, user_context.config->loglevel);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
}

/**
 * Test that a loglevel setting adds this data to the config.
 */
TEST(config_test, loglevel_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    user_context.config = nullptr;

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("loglevel 7", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    ASSERT_EQ(0U, user_context.errors.size());

    /* verify user config. */
    ASSERT_NE(nullptr, user_context.config);
    ASSERT_EQ(nullptr, user_context.config->logdir);
    ASSERT_TRUE(user_context.config->loglevel_set);
    ASSERT_EQ(7L, user_context.config->loglevel);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
}

/**
 * Test that bad loglevel ranges raise an error.
 */
TEST(config_test, loglevel_bad_range)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    user_context.config = nullptr;

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("loglevel 15", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    ASSERT_EQ(1U, user_context.errors.size());
}

/**
 * Test that the secret parameter adds data to the config.
 */
TEST(config_test, secret_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    user_context.config = nullptr;

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("secret dir", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    ASSERT_EQ(0U, user_context.errors.size());

    /* verify user config. */
    ASSERT_NE(nullptr, user_context.config);
    ASSERT_EQ(nullptr, user_context.config->logdir);
    ASSERT_FALSE(user_context.config->loglevel_set);
    ASSERT_EQ(0L, user_context.config->loglevel);
    ASSERT_STREQ("dir", user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
}

/**
 * Test that the rootblock parameter adds data to the config.
 */
TEST(config_test, rootblock_conf)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    user_context.config = nullptr;

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(
        nullptr,
        state = yy_scan_string("rootblock root", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    ASSERT_EQ(0U, user_context.errors.size());

    /* verify user config. */
    ASSERT_NE(nullptr, user_context.config);
    ASSERT_EQ(nullptr, user_context.config->logdir);
    ASSERT_FALSE(user_context.config->loglevel_set);
    ASSERT_EQ(0L, user_context.config->loglevel);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_STREQ("root", user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
}

/**
 * Test that the datastroe parameter adds data to the config.
 */
TEST(config_test, datastore_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    user_context.config = nullptr;

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("datastore data", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    ASSERT_EQ(0U, user_context.errors.size());

    /* verify user config. */
    ASSERT_NE(nullptr, user_context.config);
    ASSERT_EQ(nullptr, user_context.config->logdir);
    ASSERT_FALSE(user_context.config->loglevel_set);
    ASSERT_EQ(0L, user_context.config->loglevel);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_STREQ("data", user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
}

/**
 * Test that a single listen parameter is added to the config.
 */
TEST(config_test, listen_single)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    user_context.config = nullptr;

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("listen 0.0.0.0:1234", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    ASSERT_EQ(0U, user_context.errors.size());

    /* verify user config. */
    ASSERT_NE(nullptr, user_context.config);
    ASSERT_EQ(nullptr, user_context.config->logdir);
    ASSERT_FALSE(user_context.config->loglevel_set);
    ASSERT_EQ(0L, user_context.config->loglevel);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* check listeners. */
    ASSERT_NE(nullptr, user_context.config->listen_head);
    EXPECT_EQ(0UL, user_context.config->listen_head->addr->s_addr);
    EXPECT_EQ(1234, user_context.config->listen_head->port);
    ASSERT_EQ(nullptr, user_context.config->listen_head->hdr.next);
}

/**
 * Test that multiple config parameters are added to the config.
 */
TEST(config_test, listen_double)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    user_context.config = nullptr;

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(
        nullptr,
        state = yy_scan_string(
            "listen 0.0.0.0:1234\n"
            "listen 1.2.3.4:4321\n",
            scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    ASSERT_EQ(0U, user_context.errors.size());

    /* verify user config. */
    ASSERT_NE(nullptr, user_context.config);
    ASSERT_EQ(nullptr, user_context.config->logdir);
    ASSERT_FALSE(user_context.config->loglevel_set);
    ASSERT_EQ(0L, user_context.config->loglevel);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* check listeners. */
    config_listen_address_t* listen = user_context.config->listen_head;
    ASSERT_NE(nullptr, listen);
    EXPECT_EQ(0x04030201UL, listen->addr->s_addr);
    EXPECT_EQ(4321, listen->port);
    listen = (config_listen_address_t*)listen->hdr.next;
    ASSERT_NE(nullptr, listen);
    EXPECT_EQ(0UL, listen->addr->s_addr);
    EXPECT_EQ(1234, listen->port);
}

/**
 * Test that a chroot parameter is added to the config.
 */
TEST(config_test, chroot_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    user_context.config = nullptr;

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("chroot root", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    ASSERT_EQ(0U, user_context.errors.size());

    /* verify user config. */
    ASSERT_NE(nullptr, user_context.config);
    ASSERT_EQ(nullptr, user_context.config->logdir);
    ASSERT_FALSE(user_context.config->loglevel_set);
    ASSERT_EQ(0L, user_context.config->loglevel);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_STREQ("root", user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
}

/**
 * Test that a usergroup parameter is added to the config.
 */
TEST(config_test, usergroup_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    user_context.config = nullptr;

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("usergroup foo:bar", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    ASSERT_EQ(0U, user_context.errors.size());

    /* verify user config. */
    ASSERT_NE(nullptr, user_context.config);
    ASSERT_EQ(nullptr, user_context.config->logdir);
    ASSERT_FALSE(user_context.config->loglevel_set);
    ASSERT_EQ(0L, user_context.config->loglevel);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_NE(nullptr, user_context.config->usergroup);
    ASSERT_STREQ("foo", user_context.config->usergroup->user);
    ASSERT_STREQ("bar", user_context.config->usergroup->group);
}
