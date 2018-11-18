/**
 * \file test_dataservice_isolation_helpers.cpp
 *
 * Helpers for the dataservice isolation test.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include "../../src/dataservice/dataservice_internal.h"
#include "test_dataservice_isolation.h"

using namespace std;

const uint8_t dataservice_isolation_test::dir_key[32] = {
    0x7e, 0x4b, 0xb1, 0x5d, 0xb5, 0x00, 0x41, 0x95,
    0xb0, 0xed, 0x43, 0x59, 0x43, 0x20, 0x9b, 0x72,
    0x28, 0x07, 0xad, 0xbb, 0x87, 0x70, 0x49, 0x8a,
    0xac, 0x89, 0x44, 0xcb, 0x23, 0x56, 0x67, 0x3f
};

/**
 * \brief Dispose a test context.
 */
void dataservice_isolation_test::test_context_dispose(void* disp)
{
    test_context* ctx = (test_context*)disp;

    if (nullptr != ctx->config)
        dispose((disposable_t*)ctx->config);
}

/**
 * \brief Initialize a test_context structure.
 */
void dataservice_isolation_test::test_context_init(test_context* ctx)
{
    ctx->hdr.dispose = &test_context_dispose;
    ctx->config = nullptr;
}

/**
 * \brief Simple error setting override.
 */
void dataservice_isolation_test::set_error(
    config_context_t* context, const char* msg)
{
    test_context* ctx = (test_context*)context->user_context;

    ctx->errors.push_back(msg);
}

/**
 * \brief Simple value setting override.
 */
void dataservice_isolation_test::config_callback(
    config_context_t* context, agent_config_t* config)
{
    test_context* ctx = (test_context*)context->user_context;

    ctx->config = config;
}

void dataservice_isolation_test::SetUp()
{
    /* create bootstrap config. */
    bootstrap_config_init(&bconf);

    /* set up the parser context. */
    test_context_init(&user_context);
    context.set_error = &set_error;
    context.val_callback = &config_callback;
    context.user_context = &user_context;
    yylex_init(&scanner);
    state = yy_scan_string("", scanner);
    yyparse(scanner, &context);

    /* set the path for running agentd. */
    getcwd(wd, sizeof(wd));
    oldpath = getenv("PATH");
    if (NULL != oldpath)
    {
        path =
            strcatv(wd, "/build/host/release/bin", ":", oldpath, NULL);
    }
    else
    {
        path = strcatv(wd, "/build/host/release/bin");
    }

    setenv("PATH", path, 1);

    logsock = dup(STDERR_FILENO);

    /* spawn the dataservice process. */
    dataservice_proc_status =
        dataservice_proc(
            &bconf, user_context.config, logsock, &datasock, &datapid,
            false);

    /* by default, we run in blocking mode. */
    nonblockdatasock_configured = false;

    /* set up directory test helper. */
    string dbpath(wd);
    dbpath += "/build/test/isolation/databases/";
    directory_test_helper::SetUp(dir_key, dbpath.c_str());
}

void dataservice_isolation_test::TearDown()
{
    directory_test_helper::TearDown();

    /* terminate the dataservice process. */
    if (0 == dataservice_proc_status)
    {
        int status = 0;
        kill(datapid, SIGTERM);
        waitpid(datapid, &status, 0);
    }

    /* set the old path. */
    setenv("PATH", oldpath, 1);

    /* clean up. */
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
    close(logsock);
    dispose((disposable_t*)&bconf);
    dispose((disposable_t*)&user_context);
    free(path);
}

void dataservice_isolation_test::nonblockmode(
    function<void()> onRead, function<void()> onWrite)
{
    /* set the read/write callbacks. */
    this->onRead = onRead;
    this->onWrite = onWrite;

    /* handle a non-blocking event loop. */
    if (!nonblockdatasock_configured)
    {
        ipc_make_noblock(datasock, &nonblockdatasock, this);
        nonblockdatasock_configured = true;
        ipc_event_loop_init(&loop);
    }
    else
    {
        ipc_event_loop_remove(&loop, &nonblockdatasock);
    }

    ipc_set_readcb_noblock(&nonblockdatasock, &nonblock_read);
    ipc_set_writecb_noblock(&nonblockdatasock, &nonblock_write);
    ipc_event_loop_add(&loop, &nonblockdatasock);
    ipc_event_loop_run(&loop);
}

void dataservice_isolation_test::nonblock_read(
    ipc_socket_context_t*, int, void* ctx)
{
    dataservice_isolation_test* that = (dataservice_isolation_test*)ctx;

    that->onRead();
}

void dataservice_isolation_test::nonblock_write(
    ipc_socket_context_t*, int, void* ctx)
{
    dataservice_isolation_test* that = (dataservice_isolation_test*)ctx;

    that->onWrite();
}
