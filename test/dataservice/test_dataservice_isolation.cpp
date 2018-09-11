/**
 * \file test_dataservice_isolation.cpp
 *
 * Isolation tests for the data service.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/config.h>
#include <agentd/string.h>
#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <lmdb.h>
#include <string>
#include <unistd.h>
#include <vpr/disposable.h>

#include "../../src/dataservice/dataservice_internal.h"

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
 * The dataservice isolation test class deals with the drudgery of communicating
 * with the data service.  It provides a registration mechanism so that
 * data can be sent to the data service and received from the data service.
 */
class dataservice_isolation_test : public ::testing::Test {
protected:
    void SetUp() override
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
    }

    void TearDown() override
    {
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

    string make_data_dir_string(const char* dir)
    {
        string ret(wd);
        ret += "/build/test/isolation/databases/";
        ret += dir;

        return ret;
    }

    void mkdir(const char* dir)
    {
        string cmd("mkdir -p ");
        cmd += make_data_dir_string(dir);

        system(cmd.c_str());
    }

    void nonblockmode(function<void()> onRead, function<void()> onWrite)
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

    static void nonblock_read(ipc_socket_context_t*, int, void* ctx)
    {
        dataservice_isolation_test* that = (dataservice_isolation_test*)ctx;

        that->onRead();
    }

    static void nonblock_write(ipc_socket_context_t*, int, void* ctx)
    {
        dataservice_isolation_test* that = (dataservice_isolation_test*)ctx;

        that->onWrite();
    }

    bootstrap_config_t bconf;
    int datasock;
    int logsock;
    pid_t datapid;
    int dataservice_proc_status;
    char* path;
    char wd[16384];
    const char* oldpath;
    ipc_socket_context_t nonblockdatasock;
    bool nonblockdatasock_configured;
    ipc_event_loop_context_t loop;
    function<void()> onRead;
    function<void()> onWrite;

    YY_BUFFER_STATE state;
    yyscan_t scanner;
    config_context_t context;
    test_context user_context;
};

/**
 * Test that we can spawn the data service.
 */
TEST_F(dataservice_isolation_test, simple_spawn)
{
    ASSERT_EQ(0, dataservice_proc_status);
}

/**
 * Test that we can create the root instance using the BLOCKING call.
 */
TEST_F(dataservice_isolation_test, create_root_block_blocking)
{
    const char* DATADIR = "0c3fffcc-fc1a-49a2-a44b-823240931ca2";
    string datadir_complete = make_data_dir_string(DATADIR);
    uint32_t offset;
    uint32_t status;

    mkdir(DATADIR);

    ASSERT_EQ(0,
        dataservice_api_sendreq_root_context_init_block(
            datasock, datadir_complete.c_str()));
    ASSERT_EQ(0,
        dataservice_api_recvresp_root_context_init_block(
            datasock, &offset, &status));

    EXPECT_EQ(0U, offset);
    EXPECT_EQ(0U, status);
}

/**
 * Test that we can reduce root capabilities using the BLOCKING call.
 */
TEST_F(dataservice_isolation_test, reduce_root_caps_blocking)
{
    const char* DATADIR = "0c3fffcc-fc1a-49a2-a44b-823240931ca2";
    string datadir_complete = make_data_dir_string(DATADIR);
    uint32_t offset;
    uint32_t status;

    mkdir(DATADIR);

    /* open the database. */
    ASSERT_EQ(0,
        dataservice_api_sendreq_root_context_init_block(
            datasock, datadir_complete.c_str()));
    ASSERT_EQ(0,
        dataservice_api_recvresp_root_context_init_block(
            datasock, &offset, &status));

    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);

    /* create a reduced capabilities set for the root context. */
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant reducing root caps. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS);

    /* reduce root capabilities. */
    ASSERT_EQ(0,
        dataservice_api_sendreq_root_context_reduce_caps_block(
            datasock, reducedcaps, sizeof(reducedcaps)));
    ASSERT_EQ(0,
        dataservice_api_recvresp_root_context_reduce_caps_block(
            datasock, &offset, &status));

    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);

    /* explicitly deny reducing root caps. */
    BITCAP_SET_FALSE(reducedcaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS);

    /* reduce root capabilities. */
    ASSERT_EQ(0,
        dataservice_api_sendreq_root_context_reduce_caps_block(
            datasock, reducedcaps, sizeof(reducedcaps)));
    ASSERT_EQ(0,
        dataservice_api_recvresp_root_context_reduce_caps_block(
            datasock, &offset, &status));

    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);

    /* explicitly grant reducing root caps. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS);

    /* reduce root capabilities fails. */
    ASSERT_EQ(0,
        dataservice_api_sendreq_root_context_reduce_caps_block(
            datasock, reducedcaps, sizeof(reducedcaps)));
    ASSERT_EQ(0,
        dataservice_api_recvresp_root_context_reduce_caps_block(
            datasock, &offset, &status));

    ASSERT_EQ(0U, offset);
    ASSERT_NE(0U, status);
}

/**
 * Test that we can create the root instance.
 */
TEST_F(dataservice_isolation_test, create_root_block)
{
    const char* DATADIR = "664c437a-423b-4fc7-b425-24031134d3e5";
    string datadir_complete = make_data_dir_string(DATADIR);
    uint32_t offset;
    uint32_t status;
    int sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    int recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;

    mkdir(DATADIR);

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != IPC_ERROR_CODE_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, datadir_complete.c_str());
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    EXPECT_EQ(0U, offset);
    EXPECT_EQ(0U, status);
}

/**
 * Test that we can reduce root capabilities.
 */
TEST_F(dataservice_isolation_test, reduce_root_caps)
{
    const char* DATADIR = "664c437a-423b-4fc7-b425-24031134d3e5";
    string datadir_complete = make_data_dir_string(DATADIR);
    uint32_t offset;
    uint32_t status;
    int sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    int recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;

    mkdir(DATADIR);

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != IPC_ERROR_CODE_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, datadir_complete.c_str());
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    EXPECT_EQ(0U, offset);
    EXPECT_EQ(0U, status);

    /* create a reduced capabilities set for the root context. */
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant reducing root caps. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS);

    /* reduce root capabilities. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_reduce_caps(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != IPC_ERROR_CODE_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_reduce_caps(
                        &nonblockdatasock, reducedcaps, sizeof(reducedcaps));
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);

    /* explicitly deny reducing root caps. */
    BITCAP_SET_FALSE(reducedcaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS);

    /* reduce root capabilities. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_reduce_caps(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != IPC_ERROR_CODE_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_reduce_caps(
                        &nonblockdatasock, reducedcaps, sizeof(reducedcaps));
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);

    /* explicitly grant reducing root caps. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_LL_ROOT_CONTEXT_REDUCE_CAPS);

    /* reduce root capabilities fails. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_reduce_caps(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != IPC_ERROR_CODE_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_reduce_caps(
                        &nonblockdatasock, reducedcaps, sizeof(reducedcaps));
            }
        });

    /* the send and recv should have worked, but the command status is fail. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(0U, offset);
    ASSERT_NE(0U, status);
}
