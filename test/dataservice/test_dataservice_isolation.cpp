/**
 * \file test_dataservice_isolation.cpp
 *
 * Isolation tests for the data service.
 *
 * \copyright 2018-2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/dataservice/private/dataservice.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <lmdb.h>
#include <string>
#include <unistd.h>
#include <vccert/certificate_types.h>
#include <vpr/disposable.h>

#include "../../src/dataservice/dataservice_internal.h"
#include "test_dataservice_isolation.h"

using namespace std;

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
    uint32_t offset;
    uint32_t status;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    ASSERT_EQ(0,
        dataservice_api_sendreq_root_context_init_block(
            datasock, DB_PATH.c_str()));
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
    uint32_t offset;
    uint32_t status;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* open the database. */
    ASSERT_EQ(0,
        dataservice_api_sendreq_root_context_init_block(
            datasock, DB_PATH.c_str()));
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
    uint32_t offset;
    uint32_t status;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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
    uint32_t offset;
    uint32_t status;
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_reduce_caps(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_reduce_caps(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_reduce_caps(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
 * Test that we can create a child context using blocking calls.
 */
TEST_F(dataservice_isolation_test, child_context_create_close_blocking)
{
    uint32_t offset;
    uint32_t status;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* open the database. */
    ASSERT_EQ(0,
        dataservice_api_sendreq_root_context_init_block(
            datasock, DB_PATH.c_str()));
    ASSERT_EQ(0,
        dataservice_api_recvresp_root_context_init_block(
            datasock, &offset, &status));

    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);

    /* create a reduced capabilities set for the root context. */
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant creating and closing a child context. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);

    /* reduce root capabilities. */
    ASSERT_EQ(0,
        dataservice_api_sendreq_root_context_reduce_caps_block(
            datasock, reducedcaps, sizeof(reducedcaps)));
    ASSERT_EQ(0,
        dataservice_api_recvresp_root_context_reduce_caps_block(
            datasock, &offset, &status));

    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);

    /* create a child context */
    uint32_t child_context;
    ASSERT_EQ(0,
        dataservice_api_sendreq_child_context_create_block(
            datasock, reducedcaps, sizeof(reducedcaps)));
    ASSERT_EQ(0,
        dataservice_api_recvresp_child_context_create_block(
            datasock, &offset, &status, &child_context));

    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, child_context);

    /* close the child context */
    ASSERT_EQ(0,
        dataservice_api_sendreq_child_context_close_block(
            datasock, child_context));
    ASSERT_EQ(0,
        dataservice_api_recvresp_child_context_close_block(
            datasock, &offset, &status));

    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);
}

/**
 * Test that we can create a child context.
 */
TEST_F(dataservice_isolation_test, child_context_create_close)
{
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_close(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_global_settings_get(
                        &nonblockdatasock, &offset, &status, data, &data_size);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
    ASSERT_EQ(0U, data_size);
}

/**
 * Test that we can set and get a global setting value using blocking calls.
 */
TEST_F(dataservice_isolation_test, global_setting_set_get_blocking)
{
    uint32_t offset;
    uint32_t status;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* open the database. */
    ASSERT_EQ(0,
        dataservice_api_sendreq_root_context_init_block(
            datasock, DB_PATH.c_str()));
    ASSERT_EQ(0,
        dataservice_api_recvresp_root_context_init_block(
            datasock, &offset, &status));

    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);

    /* create a reduced capabilities set for the root context. */
    BITCAP(reducedcaps, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(reducedcaps);

    /* explicitly grant querying and setting global settings. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_GLOBAL_SETTING_WRITE);

    /* reduce root capabilities. */
    ASSERT_EQ(0,
        dataservice_api_sendreq_root_context_reduce_caps_block(
            datasock, reducedcaps, sizeof(reducedcaps)));
    ASSERT_EQ(0,
        dataservice_api_recvresp_root_context_reduce_caps_block(
            datasock, &offset, &status));

    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);

    /* create a child context */
    uint32_t child_context;
    ASSERT_EQ(0,
        dataservice_api_sendreq_child_context_create_block(
            datasock, reducedcaps, sizeof(reducedcaps)));
    ASSERT_EQ(0,
        dataservice_api_recvresp_child_context_create_block(
            datasock, &offset, &status, &child_context));

    ASSERT_EQ(0U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, child_context);

    /* set a global variable */
    const uint8_t val[16] = {
        0x17, 0x79, 0x6f, 0x55, 0xae, 0x43, 0x48, 0xa0,
        0x89, 0xab, 0xca, 0x05, 0xaf, 0x4b, 0x19, 0x6e
    };
    size_t val_size = sizeof(val);

    ASSERT_EQ(0,
        dataservice_api_sendreq_global_settings_set_block(
            datasock, child_context, DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION,
            val, val_size));
    ASSERT_EQ(0,
        dataservice_api_recvresp_global_settings_set_block(
            datasock, &offset, &status));

    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);

    /* query the global variable */
    uint8_t data[16];
    size_t data_size = sizeof(data);

    ASSERT_EQ(0,
        dataservice_api_sendreq_global_settings_get_block(
            datasock, child_context,
            DATASERVICE_GLOBAL_SETTING_SCHEMA_VERSION));
    ASSERT_EQ(0,
        dataservice_api_recvresp_global_settings_get_block(
            datasock, &offset, &status, data, &data_size));

    ASSERT_EQ(0U, status);
    ASSERT_EQ(data_size, val_size);
    ASSERT_EQ(0, memcmp(val, data, val_size));
}

/**
 * Test that we can set and get a global setting value.
 */
TEST_F(dataservice_isolation_test, global_setting_set_get)
{
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_global_settings_set(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_global_settings_get(
                        &nonblockdatasock, &offset, &status, data, &data_size);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_submit(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get_first(
                        &nonblockdatasock, &offset, &status, &node,
                        &txn_data, &txn_data_size);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_submit(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get(
                        &nonblockdatasock, &offset, &status, &node,
                        &txn_data, &txn_data_size);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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

/**
 * Test that we can submit a transaction, get it back, drop it, and can't get it
 * back.
 */
TEST_F(dataservice_isolation_test, txn_submit_get_drop)
{
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP);

    /* create child context. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_submit(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get(
                        &nonblockdatasock, &offset, &status, &node,
                        &txn_data, &txn_data_size);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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

    /* drop this transaction. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_drop(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_transaction_drop(
                        &nonblockdatasock, child_context, foo_key);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);

    /* query the first transaction. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get(
                        &nonblockdatasock, &offset, &status, &node,
                        &txn_data, &txn_data_size);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_transaction_get(
                        &nonblockdatasock, child_context, foo_key);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* clean up. */
    free(txn_data);
}

/**
 * Test that we can make a block by first submitting a transaction.
 */
TEST_F(dataservice_isolation_test, make_block_simple)
{
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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

    /* explicitly grant submitting and getting the first transaction, making
     * a block, reading a block, and reading an artifact. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_ARTIFACT_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_BY_HEIGHT_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);

    /* create child context. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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
    const uint8_t foo_prev[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    const uint8_t foo_artifact[16] = {
        0xc3, 0x84, 0x33, 0x0b, 0xf5, 0x0d, 0x42, 0xa2,
        0x9a, 0x52, 0xb5, 0xa4, 0xb3, 0x5b, 0xcf, 0x72
    };
    uint8_t* foo_cert = nullptr;
    size_t foo_cert_length = 0;

    /* create the foo transaction. */
    ASSERT_EQ(0,
        create_dummy_transaction(
            foo_key, foo_prev, foo_artifact, &foo_cert, &foo_cert_length));

    /* submit a transaction. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_submit(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_transaction_submit(
                        &nonblockdatasock, child_context, foo_key, foo_artifact,
                        foo_cert, foo_cert_length);
            }
        });

    /* verify that everything ran correctly. */
    ASSERT_EQ(0, sendreq_status);
    ASSERT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);

    void* txn_data = nullptr;
    size_t txn_data_size = 0U;
    void* block_data = nullptr;
    size_t block_data_size = 0U;
    uint8_t* foo_block_cert = nullptr;
    size_t foo_block_cert_length = 0;
    data_transaction_node_t node;
    data_artifact_record_t artifact_rec;
    data_block_node_t block_node;
    const uint8_t foo_block_id[16] = {
        0x5f, 0x5f, 0x5b, 0xea, 0xdb, 0xcd, 0x4c, 0xff,
        0xb3, 0x40, 0x99, 0x2e, 0x07, 0xf9, 0xc1, 0xef
    };

    /* create the block for below. */
    ASSERT_EQ(0,
        create_dummy_block_for_isolation(
            &builder_opts,
            foo_block_id, vccert_certificate_type_uuid_root_block, 1,
            &foo_block_cert, &foo_block_cert_length,
            foo_cert, foo_cert_length,
            nullptr));

    /* make a block. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_block_make(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_block_make(
                        &nonblockdatasock, child_context, foo_block_id,
                        foo_block_cert, foo_block_cert_length);
            }
        });
    /* verify that everything ran correctly. */
    ASSERT_EQ(0U, status);
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);

    /* query the first transaction. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get(
                        &nonblockdatasock, &offset, &status, &node,
                        &txn_data, &txn_data_size);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_transaction_get(
                        &nonblockdatasock, child_context, foo_key);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* query the first block. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_block_get(
                        &nonblockdatasock, &offset, &status, &block_node,
                        &block_data, &block_data_size);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_block_get(
                        &nonblockdatasock, child_context, foo_block_id);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(foo_block_cert_length, block_data_size);
    ASSERT_EQ(0, memcmp(foo_block_id, block_node.key, 16));

    /* query the block by height. */
    uint8_t height_block_id[16];
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_block_id_by_height_get(
                        &nonblockdatasock, &offset, &status, height_block_id);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_block_id_by_height_get(
                        &nonblockdatasock, child_context, 1U);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(0, memcmp(foo_block_id, height_block_id, 16));

    /* query the latest block id. */
    uint8_t latest_block_id[16];
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_latest_block_id_get(
                        &nonblockdatasock, &offset, &status, latest_block_id);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_latest_block_id_get(
                        &nonblockdatasock, child_context);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(0, memcmp(foo_block_id, latest_block_id, 16));

    /* query the artifact. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_artifact_get(
                        &nonblockdatasock, &offset, &status, &artifact_rec);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_artifact_get(
                        &nonblockdatasock, child_context, foo_artifact);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);
    ASSERT_EQ(0, memcmp(foo_artifact, artifact_rec.key, 16));
    ASSERT_EQ(0, memcmp(foo_key, artifact_rec.txn_first, 16));
    ASSERT_EQ(0, memcmp(foo_key, artifact_rec.txn_latest, 16));

    /* query the foo certificate. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    data_transaction_node_t canonized_node;
    void* canonized_data;
    size_t canonized_data_size;

    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_canonized_transaction_get(
                        &nonblockdatasock, &offset, &status, &canonized_node,
                        &canonized_data, &canonized_data_size);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_canonized_transaction_get(
                        &nonblockdatasock, child_context, foo_key);
            }
        });

    /* verify that the canonized transaction read worked. */
    ASSERT_EQ(0, sendreq_status);
    ASSERT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(0U, status);

    ASSERT_EQ(foo_cert_length, canonized_data_size);
    ASSERT_EQ(0, memcmp(foo_cert, canonized_data, canonized_data_size));
    ASSERT_EQ(0, memcmp(foo_key, canonized_node.key, sizeof(foo_key)));
    ASSERT_EQ(0, memcmp(foo_prev, canonized_node.prev, sizeof(foo_prev)));
    ASSERT_EQ(0, memcmp(foo_prev, canonized_node.next, sizeof(foo_prev)));
    ASSERT_EQ(0,
        memcmp(foo_artifact, canonized_node.artifact_id, sizeof(foo_artifact)));
    ASSERT_EQ(0,
        memcmp(foo_block_id, canonized_node.block_id, sizeof(foo_block_id)));

    /* clean up. */
    free(block_data);
    free(txn_data);
    free(foo_cert);
    free(foo_block_cert);
    free(canonized_data);
}

/**
 * Test that block get returns AGENTD_ERROR_DATASERVICE_NOT_FOUND if the block
 * is not found.
 */
TEST_F(dataservice_isolation_test, block_get_not_found)
{
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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

    /* explicitly grant reading a block. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ);

    /* create child context. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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

    size_t block_data_size = 0U;
    void* block_data = nullptr;
    data_block_node_t block_node;
    const uint8_t foo_block_id[16] = {
        0x19, 0xea, 0x58, 0x6b, 0xbd, 0x18, 0x4d, 0xab,
        0xbc, 0x36, 0x56, 0x6e, 0xa3, 0x49, 0x86, 0xc9
    };

    /* query the first block. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_block_get(
                        &nonblockdatasock, &offset, &status, &block_node,
                        &block_data, &block_data_size);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_block_get(
                        &nonblockdatasock, child_context, foo_block_id);
            }
        });

    /* verify that everything ran correctly and the block was not found. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
    ASSERT_EQ(nullptr, block_data);
    ASSERT_EQ(0U, block_data_size);
}

/**
 * Test that block get id by height returns AGENTD_ERROR_DATASERVICE_NOT_FOUND
 * if the block height is not found.
 */
TEST_F(dataservice_isolation_test, block_id_by_height_get_not_found)
{
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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

    /* explicitly grant reading a block id by height. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_BY_HEIGHT_READ);

    /* create child context. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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

    /* set up an empty block id. */
    uint8_t empty_block_id[16];
    memset(empty_block_id, 0, sizeof(empty_block_id));

    /* set the block id to something unexpected. */
    uint8_t height_block_id[16];
    memset(height_block_id, 0xFE, sizeof(height_block_id));

    /* query the block by height. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_block_id_by_height_get(
                        &nonblockdatasock, &offset, &status, height_block_id);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_block_id_by_height_get(
                        &nonblockdatasock, child_context, 1U);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that latest block id get returns AGENTD_ERROR_DATASERVICE_NOT_FOUND
 * if the latest block id is not found.
 */
TEST_F(dataservice_isolation_test, latest_block_id_get_not_found)
{
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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

    /* explicitly grant reading the latest block id. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);

    /* create child context. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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

    /* set up an empty block id. */
    uint8_t empty_block_id[16];
    memset(empty_block_id, 0, sizeof(empty_block_id));

    /* set the block id to something unexpected. */
    uint8_t latest_block_id[16];
    memset(latest_block_id, 0xFE, sizeof(latest_block_id));

    /* query the block by height. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_latest_block_id_get(
                        &nonblockdatasock, &offset, &status, latest_block_id);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_latest_block_id_get(
                        &nonblockdatasock, child_context);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
    ASSERT_EQ(0, memcmp(empty_block_id, latest_block_id, 16));
}

/**
 * Test that attempting to read an artifact that does not exist returns
 * AGENTD_ERROR_DATASERVICE_NOT_FOUND.
 */
TEST_F(dataservice_isolation_test, artifact_get_not_found)
{
    uint32_t offset;
    uint32_t status;
    uint32_t child_context;
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    string DB_PATH;

    /* create the directory for this test. */
    ASSERT_EQ(0, createDirectoryName(__COUNTER__, DB_PATH));

    /* Run the send / receive on creating the root context. */
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_root_context_init(
                        &nonblockdatasock, &offset, &status);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_root_context_init(
                        &nonblockdatasock, DB_PATH.c_str());
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

    /* explicitly grant reading an artifact. */
    BITCAP_SET_TRUE(reducedcaps,
        DATASERVICE_API_CAP_APP_ARTIFACT_READ);

    /* create child context. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child_context);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
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

    /* non-existent artifact id. */
    data_artifact_record_t artifact_rec;
    uint8_t foo_artifact[16] = {
        0x93, 0x0d, 0xca, 0xcf, 0x2d, 0x06, 0x4a, 0xb5,
        0x8b, 0xcc, 0xcd, 0x3e, 0x93, 0x8c, 0x03, 0xd1
    };

    /* query a non-existent artifact. */
    sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_artifact_get(
                        &nonblockdatasock, &offset, &status, &artifact_rec);

                if (recvresp_status != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite. */
        [&]() {
            if (sendreq_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                sendreq_status =
                    dataservice_api_sendreq_artifact_get(
                        &nonblockdatasock, child_context, foo_artifact);
            }
        });

    /* verify that everything ran correctly. */
    EXPECT_EQ(0, sendreq_status);
    EXPECT_EQ(0, recvresp_status);
    ASSERT_EQ(DATASERVICE_MAX_CHILD_CONTEXTS - 1U, offset);
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}
