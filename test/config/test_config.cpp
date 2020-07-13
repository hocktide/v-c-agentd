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
#include <vpr/disposable.h>

extern "C" {
#include "agentd.tab.h"
#include "agentd.yy.h"
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
 * Test that an empty config file produces a blank config.
 */
TEST(config_test, empty_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

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
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
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

    test_context_init(&user_context);

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
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a dot path logdir setting adds this data to the config.
 */
TEST(config_test, logdir_dotpath_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("logdir ./log", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there are no errors. */
    ASSERT_EQ(0U, user_context.errors.size());

    /* verify user config. */
    ASSERT_NE(nullptr, user_context.config);
    ASSERT_STREQ("./log", user_context.config->logdir);
    ASSERT_FALSE(user_context.config->loglevel_set);
    ASSERT_EQ(0L, user_context.config->loglevel);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that an absolute path for log is not accepted.
 */
TEST(config_test, logdir_no_absolute)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("logdir /log", scanner));
    ASSERT_EQ(1, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a relative path starting with .. for log is not accepted.
 */
TEST(config_test, logdir_no_dotdot)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("logdir ../log", scanner));
    ASSERT_EQ(1, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
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

    test_context_init(&user_context);

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
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
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

    test_context_init(&user_context);

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

    dispose((disposable_t*)&user_context);
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

    test_context_init(&user_context);

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
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the secret parameter can be a dot path.
 */
TEST(config_test, secret_dotpath_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("secret ./dir", scanner));
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
    ASSERT_STREQ("./dir", user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the secret parameter can't be absolute.
 */
TEST(config_test, secret_no_absolute)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("secret /dir", scanner));
    ASSERT_EQ(1, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the secret parameter can't be a dotdot relative path.
 */
TEST(config_test, secret_no_dotdot)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("secret ../dir", scanner));
    ASSERT_EQ(1, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
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

    test_context_init(&user_context);

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
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a rootblock path can be parsed.
 */
TEST(config_test, rootblock_path_conf)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(
        nullptr,
        state = yy_scan_string("rootblock root/root.cert", scanner));
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
    ASSERT_STREQ("root/root.cert", user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a rootblock dot path can be parsed.
 */
TEST(config_test, rootblock_dot_path_conf)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(
        nullptr,
        state = yy_scan_string("rootblock ./root/root.cert", scanner));
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
    ASSERT_STREQ("./root/root.cert", user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that relative paths starting with .. are not allowed.
 */
TEST(config_test, rootblock_no_dotdot)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(
        nullptr,
        state = yy_scan_string("rootblock ../root/root.cert", scanner));
    ASSERT_EQ(1, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that no absolute paths are allowed in rootblock.
 */
TEST(config_test, rootblock_no_absolute)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(
        nullptr,
        state = yy_scan_string("rootblock /root/root.cert", scanner));
    ASSERT_EQ(1, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the datastore parameter adds data to the config.
 */
TEST(config_test, datastore_config)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

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
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the datastore parameter can be a dot path.
 */
TEST(config_test, datastore_dotpath)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("datastore ./data", scanner));
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
    ASSERT_STREQ("./data", user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the datastore parameter can't be absolute.
 */
TEST(config_test, datastore_no_absolute)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("datastore /data", scanner));
    ASSERT_EQ(1, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the datastore parameter can't be a dotdot relative path.
 */
TEST(config_test, datastore_no_dotdot)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("datastore ../data", scanner));
    ASSERT_EQ(1, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
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

    test_context_init(&user_context);

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
    ASSERT_EQ(nullptr, user_context.config->view_head);

    /* check listeners. */
    ASSERT_NE(nullptr, user_context.config->listen_head);
    EXPECT_EQ(0UL, user_context.config->listen_head->addr->s_addr);
    EXPECT_EQ(1234, user_context.config->listen_head->port);
    ASSERT_EQ(nullptr, user_context.config->listen_head->hdr.next);

    dispose((disposable_t*)&user_context);
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

    test_context_init(&user_context);

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
    ASSERT_EQ(nullptr, user_context.config->view_head);

    /* check listeners. */
    config_listen_address_t* listen = user_context.config->listen_head;
    ASSERT_NE(nullptr, listen);
    EXPECT_EQ(0x04030201UL, listen->addr->s_addr);
    EXPECT_EQ(4321, listen->port);
    listen = (config_listen_address_t*)listen->hdr.next;
    ASSERT_NE(nullptr, listen);
    EXPECT_EQ(0UL, listen->addr->s_addr);
    EXPECT_EQ(1234, listen->port);

    dispose((disposable_t*)&user_context);
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

    test_context_init(&user_context);

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
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a chroot parameter can be a dot relative path.
 */
TEST(config_test, chroot_dot)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("chroot ./root", scanner));
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
    ASSERT_STREQ("./root", user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a chroot parameter can't be an absolute path.
 */
TEST(config_test, chroot_no_absolute)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("chroot /root", scanner));
    ASSERT_EQ(1, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a chroot parameter can't be a dotdot relative path.
 */
TEST(config_test, chroot_no_dotdot)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("chroot ../root", scanner));
    ASSERT_EQ(1, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    dispose((disposable_t*)&user_context);
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

    test_context_init(&user_context);

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
    ASSERT_EQ(nullptr, user_context.config->view_head);
    ASSERT_NE(nullptr, user_context.config->usergroup);
    ASSERT_STREQ("foo", user_context.config->usergroup->user);
    ASSERT_STREQ("bar", user_context.config->usergroup->group);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a canonization block parameter is accepted.
 */
TEST(config_test, empty_canonization_block)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr, state = yy_scan_string("canonization { }", scanner));
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
    ASSERT_FALSE(user_context.config->block_max_milliseconds_set);
    ASSERT_FALSE(user_context.config->block_max_transactions_set);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the block max milliseconds can be overridden.
 */
TEST(config_test, block_max_milliseconds)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state =
            yy_scan_string("canonization { max milliseconds 995 }", scanner));
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
    ASSERT_TRUE(user_context.config->block_max_milliseconds_set);
    ASSERT_EQ(995, user_context.config->block_max_milliseconds);
    ASSERT_FALSE(user_context.config->block_max_transactions_set);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a negative block max milliseconds is invalid.
 */
TEST(config_test, block_max_milliseconds_negative)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state =
            yy_scan_string("canonization { max milliseconds -7 }", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    ASSERT_EQ(1U, user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that too large of a block max milliseconds is invalid.
 */
TEST(config_test, block_max_milliseconds_large)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state =
            yy_scan_string(
                "canonization { max milliseconds 9999999999 }", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    ASSERT_EQ(1U, user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that the block max transactions can be overridden.
 */
TEST(config_test, block_max_transactions)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state =
            yy_scan_string("canonization { max transactions 17 }", scanner));
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
    ASSERT_FALSE(user_context.config->block_max_milliseconds_set);
    ASSERT_TRUE(user_context.config->block_max_transactions_set);
    ASSERT_EQ(17, user_context.config->block_max_transactions);
    ASSERT_EQ(nullptr, user_context.config->secret);
    ASSERT_EQ(nullptr, user_context.config->rootblock);
    ASSERT_EQ(nullptr, user_context.config->datastore);
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);
    ASSERT_EQ(nullptr, user_context.config->view_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that a negative block max transactions is invalid.
 */
TEST(config_test, block_max_milliseconds_transactions)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state =
            yy_scan_string("canonization { max transactions -19 }", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    ASSERT_EQ(1U, user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that too large of a block max transactions is invalid.
 */
TEST(config_test, block_max_transactions_large)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state =
            yy_scan_string(
                "canonization { max transactions 9999999 }", scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    ASSERT_EQ(1U, user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a materialized view section.
 */
TEST(config_test, empty_materialized_view)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state =
            yy_scan_string("materialized view auth { }", scanner));
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

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);
    /* there should be no artifact types set. */
    EXPECT_EQ(nullptr, user_context.config->view_head->artifact_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that duplicate view names causes an error.
 */
TEST(config_test, duplicate_materialized_view)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state =
            yy_scan_string(
                "materialized view auth { } "
                "materialized view auth { }",
                scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    ASSERT_EQ(1U, user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add an artifact type section.
 */
TEST(config_test, empty_artifact_type)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));
    /* there should be no transaction types. */
    ASSERT_EQ(nullptr, artifact->transaction_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that duplicate artifact types cause an error.
 */
TEST(config_test, duplicate_artifact_type)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { } "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { } "
            " }",
            scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there is one error. */
    ASSERT_EQ(1U, user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a transaction type section.
 */
TEST(config_test, empty_transaction_type)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { }"
                " }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    ASSERT_NE(nullptr, transaction);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, transaction->hdr.next);
    /* the transaction type should be set. */
    EXPECT_EQ(0,
        memcmp(&transaction->transaction_type, &TRANSACTION_TYPE,
               sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    EXPECT_EQ(0U, transaction->artifact_crud_flags);
    /* it should have no fields. */
    EXPECT_EQ(nullptr, transaction->field_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that duplicate transaction types cause an error.
 */
TEST(config_test, duplicate_transaction_type)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { } "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { } "
                " }"
            " }",
            scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there should be 1 error. */
    ASSERT_EQ(1U, user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add an artifact create crud flag.
 */
TEST(config_test, artifact_create_crud_flag)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "artifact { create }"
                    " }"
                " }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    ASSERT_NE(nullptr, transaction);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, transaction->hdr.next);
    /* the transaction type should be set. */
    EXPECT_EQ(0,
        memcmp(&transaction->transaction_type, &TRANSACTION_TYPE,
               sizeof(vpr_uuid)));
    /* the CREATE crud flag should be set. */
    EXPECT_EQ(MATERIALIZED_VIEW_CRUD_CREATE, transaction->artifact_crud_flags);
    /* it should have no fields. */
    EXPECT_EQ(nullptr, transaction->field_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add an artifact update crud flag.
 */
TEST(config_test, artifact_update_crud_flag)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "artifact { update }"
                    " }"
                " }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    ASSERT_NE(nullptr, transaction);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, transaction->hdr.next);
    /* the transaction type should be set. */
    EXPECT_EQ(0,
        memcmp(&transaction->transaction_type, &TRANSACTION_TYPE,
               sizeof(vpr_uuid)));
    /* the UPDATE crud flag should be set. */
    EXPECT_EQ(MATERIALIZED_VIEW_CRUD_UPDATE, transaction->artifact_crud_flags);
    /* it should have no fields. */
    EXPECT_EQ(nullptr, transaction->field_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add an artifact append crud flag.
 */
TEST(config_test, artifact_append_crud_flag)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "artifact { append }"
                    " }"
                " }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    ASSERT_NE(nullptr, transaction);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, transaction->hdr.next);
    /* the transaction type should be set. */
    EXPECT_EQ(0,
        memcmp(&transaction->transaction_type, &TRANSACTION_TYPE,
               sizeof(vpr_uuid)));
    /* the APPEND crud flag should be set. */
    EXPECT_EQ(MATERIALIZED_VIEW_CRUD_APPEND, transaction->artifact_crud_flags);
    /* it should have no fields. */
    EXPECT_EQ(nullptr, transaction->field_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add an artifact delete crud flag.
 */
TEST(config_test, artifact_delete_crud_flag)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "artifact { delete }"
                    " }"
                " }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    ASSERT_NE(nullptr, transaction);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, transaction->hdr.next);
    /* the transaction type should be set. */
    EXPECT_EQ(0,
        memcmp(&transaction->transaction_type, &TRANSACTION_TYPE,
               sizeof(vpr_uuid)));
    /* the DELETE crud flag should be set. */
    EXPECT_EQ(MATERIALIZED_VIEW_CRUD_DELETE, transaction->artifact_crud_flags);
    /* it should have no fields. */
    EXPECT_EQ(nullptr, transaction->field_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can mix artifact crud flags.
 */
TEST(config_test, artifact_mix_crud_flags)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "artifact { create update append delete }"
                    " }"
                " }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    ASSERT_NE(nullptr, transaction);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, transaction->hdr.next);
    /* the transaction type should be set. */
    EXPECT_EQ(0,
        memcmp(&transaction->transaction_type, &TRANSACTION_TYPE,
               sizeof(vpr_uuid)));
    /* the CREATE UPDATE APPEND and DELETE crud flags should be set. */
    EXPECT_EQ(
          MATERIALIZED_VIEW_CRUD_CREATE | MATERIALIZED_VIEW_CRUD_UPDATE
        | MATERIALIZED_VIEW_CRUD_APPEND | MATERIALIZED_VIEW_CRUD_DELETE,
        transaction->artifact_crud_flags);
    /* it should have no fields. */
    EXPECT_EQ(nullptr, transaction->field_head);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a field type section.
 */
TEST(config_test, empty_field_type)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;
    vpr_uuid FIELD_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &FIELD_TYPE, "ba23438b-59b9-4816-83fd-63fa6f936668"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { }"
                    " }"
                " }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    ASSERT_NE(nullptr, transaction);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, transaction->hdr.next);
    /* the transaction type should be set. */
    EXPECT_EQ(0,
        memcmp(&transaction->transaction_type, &TRANSACTION_TYPE,
               sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    EXPECT_EQ(0U, transaction->artifact_crud_flags);

    /* a field type should be populated. */
    auto field = transaction->field_head;
    ASSERT_NE(nullptr, field);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, field->hdr.next);
    /* the field code should be set. */
    EXPECT_EQ(0,
        memcmp(&field->field_code, &FIELD_TYPE, sizeof(vpr_uuid)));
    /* the short code should be 0. */
    EXPECT_EQ(0U, field->short_code);
    /* the crud flags should be 0. */
    EXPECT_EQ(0U, field->field_crud_flags);

    dispose((disposable_t*)&user_context);
}

/**
 * A duplicate field type should cause an error.
 */
TEST(config_test, duplicate_field_type)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { } "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { } "
                    " }"
                " }"
            " }",
            scanner));
    ASSERT_EQ(0, yyparse(scanner, &context));
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* there should be one error. */
    ASSERT_EQ(1U, user_context.errors.size());

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a field create crud flag.
 */
TEST(config_test, field_create_crud)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;
    vpr_uuid FIELD_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &FIELD_TYPE, "ba23438b-59b9-4816-83fd-63fa6f936668"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { "
                            "create"
                        " }"
                    " }"
                " }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    ASSERT_NE(nullptr, transaction);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, transaction->hdr.next);
    /* the transaction type should be set. */
    EXPECT_EQ(0,
        memcmp(&transaction->transaction_type, &TRANSACTION_TYPE,
               sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    EXPECT_EQ(0U, transaction->artifact_crud_flags);

    /* a field type should be populated. */
    auto field = transaction->field_head;
    ASSERT_NE(nullptr, field);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, field->hdr.next);
    /* the field code should be set. */
    EXPECT_EQ(0,
        memcmp(&field->field_code, &FIELD_TYPE, sizeof(vpr_uuid)));
    /* the short code should be 0. */
    EXPECT_EQ(0U, field->short_code);
    /* the CREATE crud flag should be set. */
    EXPECT_EQ(MATERIALIZED_VIEW_CRUD_CREATE, field->field_crud_flags);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a field update crud flag.
 */
TEST(config_test, field_update_crud)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;
    vpr_uuid FIELD_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &FIELD_TYPE, "ba23438b-59b9-4816-83fd-63fa6f936668"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { "
                            "update"
                        " }"
                    " }"
                " }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    ASSERT_NE(nullptr, transaction);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, transaction->hdr.next);
    /* the transaction type should be set. */
    EXPECT_EQ(0,
        memcmp(&transaction->transaction_type, &TRANSACTION_TYPE,
               sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    EXPECT_EQ(0U, transaction->artifact_crud_flags);

    /* a field type should be populated. */
    auto field = transaction->field_head;
    ASSERT_NE(nullptr, field);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, field->hdr.next);
    /* the field code should be set. */
    EXPECT_EQ(0,
        memcmp(&field->field_code, &FIELD_TYPE, sizeof(vpr_uuid)));
    /* the short code should be 0. */
    EXPECT_EQ(0U, field->short_code);
    /* the UPDATE crud flag should be set. */
    EXPECT_EQ(MATERIALIZED_VIEW_CRUD_UPDATE, field->field_crud_flags);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a field append crud flag.
 */
TEST(config_test, field_append_crud)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;
    vpr_uuid FIELD_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &FIELD_TYPE, "ba23438b-59b9-4816-83fd-63fa6f936668"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { "
                            "append"
                        " }"
                    " }"
                " }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    ASSERT_NE(nullptr, transaction);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, transaction->hdr.next);
    /* the transaction type should be set. */
    EXPECT_EQ(0,
        memcmp(&transaction->transaction_type, &TRANSACTION_TYPE,
               sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    EXPECT_EQ(0U, transaction->artifact_crud_flags);

    /* a field type should be populated. */
    auto field = transaction->field_head;
    ASSERT_NE(nullptr, field);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, field->hdr.next);
    /* the field code should be set. */
    EXPECT_EQ(0,
        memcmp(&field->field_code, &FIELD_TYPE, sizeof(vpr_uuid)));
    /* the short code should be 0. */
    EXPECT_EQ(0U, field->short_code);
    /* the APPEND crud flag should be set. */
    EXPECT_EQ(MATERIALIZED_VIEW_CRUD_APPEND, field->field_crud_flags);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can add a field delete crud flag.
 */
TEST(config_test, field_delete_crud)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;
    vpr_uuid FIELD_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &FIELD_TYPE, "ba23438b-59b9-4816-83fd-63fa6f936668"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { "
                            "delete"
                        " }"
                    " }"
                " }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    ASSERT_NE(nullptr, transaction);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, transaction->hdr.next);
    /* the transaction type should be set. */
    EXPECT_EQ(0,
        memcmp(&transaction->transaction_type, &TRANSACTION_TYPE,
               sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    EXPECT_EQ(0U, transaction->artifact_crud_flags);

    /* a field type should be populated. */
    auto field = transaction->field_head;
    ASSERT_NE(nullptr, field);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, field->hdr.next);
    /* the field code should be set. */
    EXPECT_EQ(0,
        memcmp(&field->field_code, &FIELD_TYPE, sizeof(vpr_uuid)));
    /* the short code should be 0. */
    EXPECT_EQ(0U, field->short_code);
    /* the DELETE crud flag should be set. */
    EXPECT_EQ(MATERIALIZED_VIEW_CRUD_DELETE, field->field_crud_flags);

    dispose((disposable_t*)&user_context);
}

/**
 * Test that we can mix field crud flags.
 */
TEST(config_test, field_mix_crud_flags)
{
    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
    vpr_uuid ARTIFACT_TYPE;
    vpr_uuid TRANSACTION_TYPE;
    vpr_uuid FIELD_TYPE;

    test_context_init(&user_context);

    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;

    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &ARTIFACT_TYPE, "b0f827ae-6d2f-4f69-b4e4-e13659c6ac44"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &TRANSACTION_TYPE, "323cdc42-3cf1-40f8-bfb9-e6daecf57689"));
    ASSERT_EQ(
        0,
        vpr_uuid_from_string(
            &FIELD_TYPE, "ba23438b-59b9-4816-83fd-63fa6f936668"));
    ASSERT_EQ(0, yylex_init(&scanner));
    ASSERT_NE(nullptr,
        state = yy_scan_string(
            "materialized view auth { "
                "artifact type b0f827ae-6d2f-4f69-b4e4-e13659c6ac44 { "
                    "transaction type 323cdc42-3cf1-40f8-bfb9-e6daecf57689 { "
                        "field type ba23438b-59b9-4816-83fd-63fa6f936668 { "
                            "create update append delete"
                        " }"
                    " }"
                " }"
            " }",
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
    ASSERT_EQ(nullptr, user_context.config->listen_head);
    ASSERT_EQ(nullptr, user_context.config->chroot);
    ASSERT_EQ(nullptr, user_context.config->usergroup);

    /* a view entry should be populated. */
    ASSERT_NE(nullptr, user_context.config->view_head);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, user_context.config->view_head->hdr.next);
    /* the name should be set. */
    EXPECT_STREQ("auth", user_context.config->view_head->name);

    /* an artifact entry should be populated. */
    auto artifact = user_context.config->view_head->artifact_head;
    ASSERT_NE(nullptr, artifact);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, artifact->hdr.next);
    /* the type should be set. */
    EXPECT_EQ(0,
        memcmp(&artifact->artifact_type, &ARTIFACT_TYPE, sizeof(vpr_uuid)));

    /* a transaction type should be populated. */
    auto transaction = artifact->transaction_head;
    ASSERT_NE(nullptr, transaction);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, transaction->hdr.next);
    /* the transaction type should be set. */
    EXPECT_EQ(0,
        memcmp(&transaction->transaction_type, &TRANSACTION_TYPE,
               sizeof(vpr_uuid)));
    /* the crud flags should be 0. */
    EXPECT_EQ(0U, transaction->artifact_crud_flags);

    /* a field type should be populated. */
    auto field = transaction->field_head;
    ASSERT_NE(nullptr, field);
    /* it should be the only entry. */
    EXPECT_EQ(nullptr, field->hdr.next);
    /* the field code should be set. */
    EXPECT_EQ(0,
        memcmp(&field->field_code, &FIELD_TYPE, sizeof(vpr_uuid)));
    /* the short code should be 0. */
    EXPECT_EQ(0U, field->short_code);
    /* the CREATE UPDATE APPEND and DELETE crud flags should be set. */
    EXPECT_EQ(
          MATERIALIZED_VIEW_CRUD_CREATE | MATERIALIZED_VIEW_CRUD_UPDATE
        | MATERIALIZED_VIEW_CRUD_APPEND | MATERIALIZED_VIEW_CRUD_DELETE,
        field->field_crud_flags);

    dispose((disposable_t*)&user_context);
}
