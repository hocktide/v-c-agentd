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
#include <agentd/inet.h>
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

/**
 * Test that we can create a child context.
 */
TEST_F(dataservice_isolation_test, child_context_create_close)
{
    const char* DATADIR = "664c437a-423b-4fc7-b425-24031134d3e5";
    string datadir_complete = make_data_dir_string(DATADIR);
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
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

    /* create a reduced capabilities set for the child context. */
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant closing the child context. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* create child context. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

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
                    dataservice_api_sendreq_child_context_create(
                        &nonblockdatasock, reducedcaps, sizeof(reducedcaps));
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, child_context);

    /* close child context. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_close(
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
                    dataservice_api_sendreq_child_context_close(
                        &nonblockdatasock, child_context);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);
}

/**
 * Test that we can't find a global setting in an empty database.
 */
TEST_F(dataservice_isolation_test, global_setting_not_found)
{
    const char* DATADIR = "ae9148a4-ff93-4b8b-a8d6-eab8c960b694";
    string datadir_complete = make_data_dir_string(DATADIR);
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
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

    /* create a reduced capabilities set for the child context. */
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant querying global settings. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ);

    /* create child context. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

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
                    dataservice_api_sendreq_child_context_create(
                        &nonblockdatasock, reducedcaps, sizeof(reducedcaps));
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, child_context);

    char data[16];
    size_t data_size = sizeof(data);

    /* query global settings. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_global_settings_get(
                        &nonblockdatasock, &offset, &status, data, &data_size);

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
                    dataservice_api_sendreq_global_settings_get(
                        &nonblockdatasock, child_context,
                        DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    /* this will fail with not found. */
    ASSERT_EQ(1U, status);
}

/**
 * Test that we can set and get a global setting value.
 */
TEST_F(dataservice_isolation_test, global_setting_set_get)
{
    const char* DATADIR = "b43b4c1c-173f-4019-8952-f6bd24f62f44";
    string datadir_complete = make_data_dir_string(DATADIR);
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
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

    /* create a reduced capabilities set for the child context. */
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant querying and setting global settings. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE);

    /* create child context. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

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
                    dataservice_api_sendreq_child_context_create(
                        &nonblockdatasock, reducedcaps, sizeof(reducedcaps));
            }
        });

    /* verify that everything ran correctly. */
    ASSERT_EQ(0, sendreq_status);
    ASSERT_EQ(0, recvresp_status);
    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, child_context);

    const uint8_t val[16] = {
        0x17, 0x79, 0x6f, 0x55, 0xae, 0x43, 0x48, 0xa0,
        0x89, 0xab, 0xca, 0x05, 0xaf, 0x4b, 0x19, 0x6e
    };
    size_t val_size = sizeof(val);

    /* write global settings. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_global_settings_set(
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
                    dataservice_api_sendreq_global_settings_set(
                        &nonblockdatasock, child_context,
                        DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION,
                        val, val_size);
            }
        });

    /* verify that everything ran correctly. */
    ASSERT_EQ(0, sendreq_status);
    ASSERT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);

    char data[16];
    size_t data_size = sizeof(data);

    /* query global settings. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_global_settings_get(
                        &nonblockdatasock, &offset, &status, data, &data_size);

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
                    dataservice_api_sendreq_global_settings_get(
                        &nonblockdatasock, child_context,
                        DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(data_size, val_size);
    ASSERT_EQ(0, memcmp(val, data, val_size));
}

/**
 * Test that we can submit a transaction and get it back from the transaction
 * queue.
 */
TEST_F(dataservice_isolation_test, txn_submit_get_first)
{
    const char* DATADIR = "108f73a7-5d6e-4886-97e7-b6a579c7994a";
    string datadir_complete = make_data_dir_string(DATADIR);
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
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

    /* create a reduced capabilities set for the child context. */
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant submitting and getting the first transaction. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);

    /* create child context. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

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
                    dataservice_api_sendreq_child_context_create(
                        &nonblockdatasock, reducedcaps, sizeof(reducedcaps));
            }
        });

    /* verify that everything ran correctly. */
    ASSERT_EQ(0, sendreq_status);
    ASSERT_EQ(0, recvresp_status);
    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, child_context);

    const uint8_t foo_key[16] = {
        0x05, 0x09, 0x43, 0x34, 0x0f, 0xb0, 0x4a, 0xa2,
        0xa1, 0xf2, 0x26, 0x15, 0x6a, 0x56, 0x45, 0x4d
    };
    const uint8_t foo_artifact[16] = {
        0xc3, 0x84, 0x33, 0x0b, 0xf5, 0x0d, 0x42, 0xa2,
        0x9a, 0x52, 0xb5, 0xa4, 0xb3, 0x5b, 0xcf, 0x72
    };
    const uint8_t foo_data[16] = {
        0x80, 0xfb, 0x52, 0x78, 0xa0, 0x63, 0x4a, 0xf0,
        0x81, 0x56, 0xba, 0xab, 0xe5, 0xe0, 0x56, 0x68
    };
    size_t foo_data_size = sizeof(foo_data);

    /* submit a transaction. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_submit(
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
                    dataservice_api_sendreq_transaction_submit(
                        &nonblockdatasock, child_context, foo_key, foo_artifact,
                        foo_data, foo_data_size);
            }
        });

    /* verify that everything ran correctly. */
    ASSERT_EQ(0, sendreq_status);
    ASSERT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);

    void* txn_data = nullptr;
    size_t txn_data_size = 0U;
    data_transaction_node_t node;

    /* query the first transaction. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get_first(
                        &nonblockdatasock, &offset, &status, &node,
                        &txn_data, &txn_data_size);

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
                    dataservice_api_sendreq_transaction_get_first(
                        &nonblockdatasock, child_context);
            }
        });

    /* verify that everything ran correctly. */
    uint8_t begin_key[16];
    memset(begin_key, 0, sizeof(begin_key));
    uint8_t end_key[16];
    memset(end_key, 0xFF, sizeof(end_key));
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(txn_data_size, foo_data_size);
    ASSERT_EQ(0, memcmp(txn_data, foo_data, txn_data_size));
    ASSERT_EQ(0, memcmp(node.key, foo_key, sizeof(node.key)));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo_artifact, sizeof(node.artifact_id)));
    ASSERT_EQ(0, memcmp(node.prev, begin_key, sizeof(node.prev)));
    ASSERT_EQ(0, memcmp(node.next, end_key, sizeof(node.next)));
    ASSERT_EQ(foo_data_size, (uint64_t)ntohll(node.net_txn_cert_size));

    /* clean up. */
    free(txn_data);
}

/**
 * Test that we can submit a transaction and get it back from the transaction
 * queue, by ID.
 */
TEST_F(dataservice_isolation_test, txn_submit_get)
{
    const char* DATADIR = "287f4446-2ae5-4bd5-8eb1-dbceafe1f291";
    string datadir_complete = make_data_dir_string(DATADIR);
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
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

    /* create a reduced capabilities set for the child context. */
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant submitting and getting the first transaction. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);

    /* create child context. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

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
                    dataservice_api_sendreq_child_context_create(
                        &nonblockdatasock, reducedcaps, sizeof(reducedcaps));
            }
        });

    /* verify that everything ran correctly. */
    ASSERT_EQ(0, sendreq_status);
    ASSERT_EQ(0, recvresp_status);
    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, child_context);

    const uint8_t foo_key[16] = {
        0x05, 0x09, 0x43, 0x34, 0x0f, 0xb0, 0x4a, 0xa2,
        0xa1, 0xf2, 0x26, 0x15, 0x6a, 0x56, 0x45, 0x4d
    };
    const uint8_t foo_artifact[16] = {
        0xc3, 0x84, 0x33, 0x0b, 0xf5, 0x0d, 0x42, 0xa2,
        0x9a, 0x52, 0xb5, 0xa4, 0xb3, 0x5b, 0xcf, 0x72
    };
    const uint8_t foo_data[16] = {
        0x80, 0xfb, 0x52, 0x78, 0xa0, 0x63, 0x4a, 0xf0,
        0x81, 0x56, 0xba, 0xab, 0xe5, 0xe0, 0x56, 0x68
    };
    size_t foo_data_size = sizeof(foo_data);

    /* submit a transaction. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_submit(
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
                    dataservice_api_sendreq_transaction_submit(
                        &nonblockdatasock, child_context, foo_key, foo_artifact,
                        foo_data, foo_data_size);
            }
        });

    /* verify that everything ran correctly. */
    ASSERT_EQ(0, sendreq_status);
    ASSERT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);

    void* txn_data = nullptr;
    size_t txn_data_size = 0U;
    data_transaction_node_t node;

    /* query the first transaction. */
    sendreq_status = IPC_ERROR_CODE_WOULD_BLOCK;
    recvresp_status = IPC_ERROR_CODE_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == IPC_ERROR_CODE_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get(
                        &nonblockdatasock, &offset, &status, &node,
                        &txn_data, &txn_data_size);

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
                    dataservice_api_sendreq_transaction_get(
                        &nonblockdatasock, child_context, foo_key);
            }
        });

    /* verify that everything ran correctly. */
    uint8_t begin_key[16];
    memset(begin_key, 0, sizeof(begin_key));
    uint8_t end_key[16];
    memset(end_key, 0xFF, sizeof(end_key));
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(txn_data_size, foo_data_size);
    ASSERT_EQ(0, memcmp(txn_data, foo_data, txn_data_size));
    ASSERT_EQ(0, memcmp(node.key, foo_key, sizeof(node.key)));
    ASSERT_EQ(0, memcmp(node.artifact_id, foo_artifact, sizeof(node.artifact_id)));
    ASSERT_EQ(0, memcmp(node.prev, begin_key, sizeof(node.prev)));
    ASSERT_EQ(0, memcmp(node.next, end_key, sizeof(node.next)));
    ASSERT_EQ(foo_data_size, (uint64_t)ntohll(node.net_txn_cert_size));

    /* clean up. */
    free(txn_data);
}
