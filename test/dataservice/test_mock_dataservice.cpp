/**
 * \file test_mock_dataservice.cpp
 *
 * Test the mock data service private API.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <agentd/status_codes.h>
#include <ostream>
#include <vccert/certificate_types.h>

#include "test_mock_dataservice.h"
#include "../mocks/dataservice.h"

using namespace std;

/**
 * If the artifact get mock callback is not set,
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_artifact_get)
{
    data_artifact_record_t artifact_rec;
    uint8_t artifact_id[16] = {
        0x0b, 0x62, 0xf6, 0xdf, 0x44, 0xc4, 0x41, 0x3c,
        0xa7, 0xdc, 0xf2, 0x6f, 0xeb, 0x2e, 0xc6, 0x3a
    };
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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
                        &nonblockdatasock, child_context, artifact_id);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the sent artifact get request.
 */
TEST_F(mock_dataservice_test, matches_artifact_get)
{
    data_artifact_record_t artifact_rec;
    uint8_t artifact_id[16] = {
        0x0b, 0x62, 0xf6, 0xdf, 0x44, 0xc4, 0x41, 0x3c,
        0xa7, 0xdc, 0xf2, 0x6f, 0xeb, 0x2e, 0xc6, 0x3a
    };
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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
                        &nonblockdatasock, child_context, artifact_id);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_payload_artifact_read(
            child_context, artifact_id));
}

/**
 * If the artifact get mock callback is set,
 * then the status code and data it returns is returned in the API call.
 */
TEST_F(mock_dataservice_test, artifact_get_override)
{
    data_artifact_record_t artifact_rec;
    uint8_t artifact_id[16] = {
        0x0b, 0x62, 0xf6, 0xdf, 0x44, 0xc4, 0x41, 0x3c,
        0xa7, 0xdc, 0xf2, 0x6f, 0xeb, 0x2e, 0xc6, 0x3a
    };
    uint8_t txn_first_id[16] = {
        0x5e, 0x1b, 0x19, 0x43, 0x91, 0xb1, 0x4c, 0x1b,
        0x8c, 0xef, 0x01, 0x6a, 0x6a, 0x60, 0x7c, 0x69
    };
    uint8_t txn_last_id[16] = {
        0xed, 0x7f, 0x6c, 0x75, 0x44, 0xf1, 0x44, 0x7d,
        0xb0, 0xe5, 0xc8, 0x2a, 0x4a, 0xe3, 0x4c, 0x50
    };
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint64_t TXN_HEIGHT_FIRST = 17;
    const uint64_t TXN_HEIGHT_LAST = 21;
    const uint32_t ARTIFACT_STATE = 5;

    /* mock the artifact_get api call. */
    mock->register_callback_payload_artifact_read(
        [&](const dataservice_request_payload_artifact_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_payload_artifact_read(
                    &payload, &payload_size, artifact_id, txn_first_id,
                    txn_last_id, TXN_HEIGHT_FIRST, TXN_HEIGHT_LAST, ARTIFACT_STATE);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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
                        &nonblockdatasock, child_context, artifact_id);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the mock returns success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the data key matches the artifact id. */
    EXPECT_EQ(0, memcmp(artifact_id, artifact_rec.key, 16));
    /* the data first txn matches our first txn. */
    EXPECT_EQ(0, memcmp(txn_first_id, artifact_rec.txn_first, 16));
    /* the data latest txn matches our latest txn. */
    EXPECT_EQ(0, memcmp(txn_last_id, artifact_rec.txn_latest, 16));
    /* the first height matches. */
    EXPECT_EQ((uint64_t)ntohll(TXN_HEIGHT_FIRST),
        artifact_rec.net_height_first);
    /* the latest height matches. */
    EXPECT_EQ((uint64_t)ntohll(TXN_HEIGHT_LAST),
        artifact_rec.net_height_latest);
    /* the state matches. */
    EXPECT_EQ((uint32_t)ntohl(ARTIFACT_STATE), artifact_rec.net_state_latest);
}

/**
 * If the block id by height read mock is not set, then
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_block_id_by_height_read)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    uint64_t height = 777;
    uint8_t block_id[16];

    /* start the mock dataservice. */
    mock->start();

    /* precondition: block id is zeroes. */
    memset(block_id, 0, sizeof(block_id));

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_block_id_by_height_get(
                        &nonblockdatasock, &offset, &status, block_id);

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
                        &nonblockdatasock, child_context, height);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the sent block id by height read request.
 */
TEST_F(mock_dataservice_test, matches_block_id_by_height_read)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    uint64_t height = 777;
    uint8_t block_id[16];

    /* start the mock dataservice. */
    mock->start();

    /* precondition: block id is zeroes. */
    memset(block_id, 0, sizeof(block_id));

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_block_id_by_height_get(
                        &nonblockdatasock, &offset, &status, block_id);

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
                        &nonblockdatasock, child_context, height);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_block_id_by_height_read(
            child_context, height));
}

/**
 * If the block id by height read mock is set, then
 * the status code and data it returns is returned in the api call.
 */
TEST_F(mock_dataservice_test, block_id_by_height_read_override)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    uint64_t height = 777;
    uint8_t block_id[16];
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xcb, 0xd6, 0x35, 0x00, 0x75, 0x55, 0x4c, 0xa3,
        0xab, 0xbe, 0x65, 0xb1, 0xcc, 0x54, 0xf6, 0x99
    };

    /* mock the block id by height api call. */
    mock->register_callback_block_id_by_height_read(
        [&](const dataservice_request_block_id_by_height_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_id_by_height_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* precondition: block id is zeroes. */
    memset(block_id, 0, sizeof(block_id));

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_block_id_by_height_get(
                        &nonblockdatasock, &offset, &status, block_id);

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
                        &nonblockdatasock, child_context, height);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the mock returns success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the block id should be the expected block id. */
    EXPECT_EQ(0, memcmp(block_id, EXPECTED_BLOCK_ID, 16));
}

/**
 * If the block id latest read mock is not set, then
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_block_id_latest_read)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    uint8_t block_id[16];

    /* start the mock dataservice. */
    mock->start();

    /* precondition: block id is zeroes. */
    memset(block_id, 0, sizeof(block_id));

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_latest_block_id_get(
                        &nonblockdatasock, &offset, &status, block_id);

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

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the sent block id latest read request.
 */
TEST_F(mock_dataservice_test, matches_block_id_latest_read)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    uint8_t block_id[16];

    /* start the mock dataservice. */
    mock->start();

    /* precondition: block id is zeroes. */
    memset(block_id, 0, sizeof(block_id));

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_latest_block_id_get(
                        &nonblockdatasock, &offset, &status, block_id);

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

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_block_id_latest_read(
            child_context));
}

/**
 * If the block id latest read mock is set, then
 * the status code and data it returns is returned in the api cal.
 */
TEST_F(mock_dataservice_test, block_id_latest_read_override)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    uint8_t block_id[16];
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0x7e, 0xe0, 0xf5, 0xa9, 0xa1, 0x33, 0x48, 0x7e,
        0xb5, 0x0b, 0x72, 0x77, 0x78, 0x69, 0xa2, 0x55
    };

    /* mock the latest block id api call. */
    mock->register_callback_block_id_latest_read(
        [&](const dataservice_request_block_id_latest_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_id_latest_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* precondition: block id is zeroes. */
    memset(block_id, 0, sizeof(block_id));

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_latest_block_id_get(
                        &nonblockdatasock, &offset, &status, block_id);

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

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the mock returns success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the block id should be the expected block id. */
    EXPECT_EQ(0, memcmp(block_id, EXPECTED_BLOCK_ID, 16));
}

/**
 * If the block make mock is not set, then
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_block_make)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xcc, 0x05, 0x7c, 0xf1, 0xa2, 0x80, 0x45, 0x33,
        0x8f, 0xd4, 0x5a, 0xfd, 0x71, 0xd1, 0x5f, 0x38
    };
    const uint8_t EXPECTED_BLOCK_CERT[4] = { 0x00, 0x01, 0x02, 0x03 };

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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
                        &nonblockdatasock, child_context, EXPECTED_BLOCK_ID,
                        EXPECTED_BLOCK_CERT, sizeof(EXPECTED_BLOCK_CERT));
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match agaist the sent block make request.
 */
TEST_F(mock_dataservice_test, matches_block_make)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xcc, 0x05, 0x7c, 0xf1, 0xa2, 0x80, 0x45, 0x33,
        0x8f, 0xd4, 0x5a, 0xfd, 0x71, 0xd1, 0x5f, 0x38
    };
    const uint8_t EXPECTED_BLOCK_CERT[4] = { 0x00, 0x01, 0x02, 0x03 };

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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
                        &nonblockdatasock, child_context, EXPECTED_BLOCK_ID,
                        EXPECTED_BLOCK_CERT, sizeof(EXPECTED_BLOCK_CERT));
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_block_make(
            child_context, EXPECTED_BLOCK_ID, sizeof(EXPECTED_BLOCK_CERT),
            EXPECTED_BLOCK_CERT));
}

/**
 * If the block make mock is set, then
 * the status code and data it returns is returned in the api call.
 */
TEST_F(mock_dataservice_test, block_make_override)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xcc, 0x05, 0x7c, 0xf1, 0xa2, 0x80, 0x45, 0x33,
        0x8f, 0xd4, 0x5a, 0xfd, 0x71, 0xd1, 0x5f, 0x38
    };
    const uint8_t EXPECTED_BLOCK_CERT[4] = { 0x00, 0x01, 0x02, 0x03 };

    /* mock the block make api call. */
    mock->register_callback_block_make(
        [&](const dataservice_request_block_make_t&,
            std::ostream&) {
            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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
                        &nonblockdatasock, child_context, EXPECTED_BLOCK_ID,
                        EXPECTED_BLOCK_CERT, sizeof(EXPECTED_BLOCK_CERT));
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the mock returns success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
}

/**
 * If the block read mock is not set, then
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_block_read)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0x77, 0xee, 0xdd, 0xe5, 0xf7, 0x1b, 0x4f, 0x36,
        0x99, 0xdc, 0x51, 0xc7, 0x80, 0xd8, 0x63, 0x1f
    };
    data_block_node_t block_node;
    void* data = nullptr;
    size_t data_size = 0U;

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_block_get(
                        &nonblockdatasock, &offset, &status, &block_node,
                        &data, &data_size);

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
                        &nonblockdatasock, child_context, EXPECTED_BLOCK_ID);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the block read request.
 */
TEST_F(mock_dataservice_test, matches_block_read)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0x77, 0xee, 0xdd, 0xe5, 0xf7, 0x1b, 0x4f, 0x36,
        0x99, 0xdc, 0x51, 0xc7, 0x80, 0xd8, 0x63, 0x1f
    };
    data_block_node_t block_node;
    void* data = nullptr;
    size_t data_size = 0U;

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_block_get(
                        &nonblockdatasock, &offset, &status, &block_node,
                        &data, &data_size);

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
                        &nonblockdatasock, child_context, EXPECTED_BLOCK_ID);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_block_read(
            child_context, EXPECTED_BLOCK_ID));
}

/**
 * If the block read mock is set, then
 * the status code and data it returns is returned in the api call.
 */
TEST_F(mock_dataservice_test, block_read_override)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0x77, 0xee, 0xdd, 0xe5, 0xf7, 0x1b, 0x4f, 0x36,
        0x99, 0xdc, 0x51, 0xc7, 0x80, 0xd8, 0x63, 0x1f
    };
    const uint8_t EXPECTED_PREV_ID[16] = {
        0x1b, 0xfb, 0x0e, 0x12, 0xb1, 0x3b, 0x4e, 0x36,
        0x93, 0x61, 0xb2, 0x6e, 0x0f, 0xcb, 0x7c, 0x67
    };
    const uint8_t EXPECTED_NEXT_ID[16] = {
        0x36, 0xc8, 0x36, 0x5e, 0x84, 0x71, 0x46, 0x8b,
        0x84, 0xb7, 0x3e, 0xe4, 0x0e, 0x2b, 0x5e, 0x94
    };
    const uint8_t EXPECTED_FIRST_TXN_ID[16] = {
        0x8d, 0xdd, 0x0f, 0x67, 0xe8, 0x43, 0x4c, 0x1f,
        0xa2, 0x2e, 0x2a, 0x39, 0xe3, 0x52, 0x84, 0x78
    };
    const uint64_t EXPECTED_BLOCK_HEIGHT = 76;
    const uint8_t EXPECTED_CERT[4] = { 0x00, 0x01, 0x02, 0x03 };
    const size_t EXPECTED_CERT_SIZE = sizeof(EXPECTED_CERT);
    data_block_node_t block_node;
    void* data = nullptr;
    size_t data_size = 0U;

    /* mock the block read api call. */
    mock->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID, EXPECTED_PREV_ID,
                    EXPECTED_NEXT_ID, EXPECTED_FIRST_TXN_ID, EXPECTED_BLOCK_HEIGHT,
                    EXPECTED_CERT, EXPECTED_CERT_SIZE);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_block_get(
                        &nonblockdatasock, &offset, &status, &block_node,
                        &data, &data_size);

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
                        &nonblockdatasock, child_context, EXPECTED_BLOCK_ID);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the mock returns success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the node data is correct. */
    EXPECT_EQ(0, memcmp(EXPECTED_BLOCK_ID, block_node.key, 16));
    EXPECT_EQ(0, memcmp(EXPECTED_PREV_ID, block_node.prev, 16));
    EXPECT_EQ(0, memcmp(EXPECTED_NEXT_ID, block_node.next, 16));
    EXPECT_EQ(0,
        memcmp(EXPECTED_FIRST_TXN_ID, block_node.first_transaction_id, 16));
    EXPECT_EQ((int64_t)EXPECTED_BLOCK_HEIGHT,
        ntohll(block_node.net_block_height));
    EXPECT_EQ((int64_t)EXPECTED_CERT_SIZE,
        ntohll(block_node.net_block_cert_size));
    ASSERT_EQ(EXPECTED_CERT_SIZE, data_size);
    ASSERT_NE(nullptr, data);
    EXPECT_EQ(0, memcmp(EXPECTED_CERT, data, data_size));
}

/**
 * If the canonized transaction get mock is not set,
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_canonized_transaction_get)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x82, 0xfd, 0xa8, 0xd1, 0x6e, 0x45, 0x4e, 0xbf,
        0xae, 0x32, 0xc9, 0xf0, 0x8a, 0x4b, 0x0a, 0xeb
    };
    data_transaction_node_t txn_node;
    void* data = nullptr;
    size_t data_size = 0U;

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_canonized_transaction_get(
                        &nonblockdatasock, &offset, &status, &txn_node,
                        &data, &data_size);

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
                        &nonblockdatasock, child_context, EXPECTED_TXN_ID);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the sent canonized transaction get request.
 */
TEST_F(mock_dataservice_test, matches_canonized_transaction_get)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x82, 0xfd, 0xa8, 0xd1, 0x6e, 0x45, 0x4e, 0xbf,
        0xae, 0x32, 0xc9, 0xf0, 0x8a, 0x4b, 0x0a, 0xeb
    };
    data_transaction_node_t txn_node;
    void* data = nullptr;
    size_t data_size = 0U;

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_canonized_transaction_get(
                        &nonblockdatasock, &offset, &status, &txn_node,
                        &data, &data_size);

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
                        &nonblockdatasock, child_context, EXPECTED_TXN_ID);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_canonized_transaction_get(
            child_context, EXPECTED_TXN_ID));
}

/**
 * If the canonized transaction get mock is set,
 * the status code and data it returns is returned in the api call.
 */
TEST_F(mock_dataservice_test, canonized_transaction_get_override)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x82, 0xfd, 0xa8, 0xd1, 0x6e, 0x45, 0x4e, 0xbf,
        0xae, 0x32, 0xc9, 0xf0, 0x8a, 0x4b, 0x0a, 0xeb
    };
    const uint8_t EXPECTED_PREV_ID[16] = {
        0x99, 0x57, 0xff, 0x00, 0xc2, 0xf9, 0x4a, 0x79,
        0x8a, 0xac, 0x76, 0x0a, 0x01, 0xe7, 0xd2, 0xd2
    };
    const uint8_t EXPECTED_NEXT_ID[16] = {
        0xc4, 0xb1, 0xc7, 0xe2, 0xe8, 0x94, 0x4a, 0x0f,
        0x82, 0xac, 0x6c, 0x21, 0xdc, 0xc7, 0x77, 0x08
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xad, 0x61, 0x60, 0xc3, 0x6f, 0x36, 0x45, 0x9a,
        0xb2, 0x28, 0xb4, 0xeb, 0x0a, 0x3b, 0xc7, 0x13
    };
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0x5e, 0x8d, 0x0f, 0x2e, 0xfa, 0x15, 0x41, 0x7f,
        0x9d, 0x30, 0xf5, 0x45, 0x91, 0x7c, 0x57, 0xa8
    };
    const uint8_t EXPECTED_CERT[5] = { 0x05, 0x04, 0x03, 0x02, 0x01 };
    const size_t EXPECTED_CERT_SIZE = sizeof(EXPECTED_CERT);
    data_transaction_node_t txn_node;
    void* data = nullptr;
    size_t data_size = 0U;

    /* mock the canonized transaction read api call. */
    mock->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_PREV_ID,
                    EXPECTED_NEXT_ID, EXPECTED_ARTIFACT_ID, EXPECTED_BLOCK_ID,
                    EXPECTED_CERT, EXPECTED_CERT_SIZE);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_canonized_transaction_get(
                        &nonblockdatasock, &offset, &status, &txn_node,
                        &data, &data_size);

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
                        &nonblockdatasock, child_context, EXPECTED_TXN_ID);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the mock returns success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the node data is correct. */
    EXPECT_EQ(0, memcmp(EXPECTED_TXN_ID, txn_node.key, 16));
    EXPECT_EQ(0, memcmp(EXPECTED_PREV_ID, txn_node.prev, 16));
    EXPECT_EQ(0, memcmp(EXPECTED_NEXT_ID, txn_node.next, 16));
    EXPECT_EQ(0, memcmp(EXPECTED_ARTIFACT_ID, txn_node.artifact_id, 16));
    EXPECT_EQ(0, memcmp(EXPECTED_BLOCK_ID, txn_node.block_id, 16));
    EXPECT_EQ((int64_t)EXPECTED_CERT_SIZE, ntohll(txn_node.net_txn_cert_size));
    ASSERT_EQ(EXPECTED_CERT_SIZE, data_size);
    ASSERT_NE(nullptr, data);
    EXPECT_EQ(0, memcmp(EXPECTED_CERT, data, data_size));
}

/**
 * If the child context close mock is not set,
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_child_context_close)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the sent artifact get request.
 */
TEST_F(mock_dataservice_test, matches_child_context_close)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_child_context_close(
            child_context));
}

/**
 * If the child context close mock is set,
 * the status code and data it returns is returned in the api call.
 */
TEST_F(mock_dataservice_test, child_context_close_override)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;

    /* mock the child context close api call. */
    mock->register_callback_child_context_close(
        [&](const dataservice_request_child_context_close_t&,
            std::ostream&) {
            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the mock returns success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
}

/**
 * If the child context create mock is not set,
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_child_context_create)
{
    uint32_t offset = 0U;
    uint32_t status = 0U;
    uint32_t child = 0U;
    BITCAP(childcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child);

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
                        &nonblockdatasock, childcaps, sizeof(childcaps));
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the sent child context create request.
 */
TEST_F(mock_dataservice_test, matches_child_context_create)
{
    uint32_t offset = 0U;
    uint32_t status = 0U;
    uint32_t child = 0U;
    BITCAP(childcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child);

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
                        &nonblockdatasock, childcaps, sizeof(childcaps));
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_child_context_create(
            childcaps));
}

/**
 * If the child context create mock is set, then
 * the status code and data it returns is returned in the api call.
 */
TEST_F(mock_dataservice_test, child_context_create_override)
{
    uint32_t offset = 0U;
    uint32_t status = 0U;
    uint32_t child = 0U;
    const uint32_t EXPECTED_CHILD = 1023U;
    BITCAP(childcaps, DATASERVICE_API_CAP_BITS_MAX);

    /* mock the child context create api call. */
    mock->register_callback_child_context_create(
        [&](const dataservice_request_child_context_create_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_child_context_create(
                    &payload, &payload_size, EXPECTED_CHILD);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_child_context_create(
                        &nonblockdatasock, &offset, &status, &child);

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
                        &nonblockdatasock, childcaps, sizeof(childcaps));
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the mock returns success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the child context is correct. */
    EXPECT_EQ(EXPECTED_CHILD, child);
}

/**
 * If the global setting mock callback is not set,
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_global_setting_get)
{
    uint32_t offset = 0U;
    uint32_t status = 0U;
    uint8_t buffer[32];
    size_t response_size = sizeof(buffer);

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send the global settings get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_api_sendreq_global_settings_get_block(
            datasock, 0U, 0U));

    /* we should be able to get a response from this request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_api_recvresp_global_settings_get_block(
            datasock, &offset, &status, buffer, &response_size));

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the sent artifact get request.
 */
TEST_F(mock_dataservice_test, matches_global_setting_get)
{
    uint32_t child_context = 17U;
    uint64_t key = 93880U;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    uint8_t buffer[32];
    size_t response_size = sizeof(buffer);

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send the global settings get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_api_sendreq_global_settings_get_block(
            datasock, child_context, key));

    /* we should be able to get a response from this request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_api_recvresp_global_settings_get_block(
            datasock, &offset, &status, buffer, &response_size));

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_global_setting_get(
            child_context, key));
}

/**
 * We can override the global setting get call to return an arbitrary value.
 */
TEST_F(mock_dataservice_test, global_setting_get_override)
{
    uint32_t offset = 0U;
    uint32_t status = 0U;
    uint8_t buffer[32];
    size_t response_size = sizeof(buffer);

    /* set the mock to write a default value. */
    mock->register_callback_global_setting_get(
        [](const dataservice_request_global_setting_get_t&, ostream& out) {
            uint64_t dummy = 321;
            out.write((const char*)&dummy, sizeof(dummy));
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send the global settings get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_api_sendreq_global_settings_get_block(
            datasock, 0U, 0U));

    /* we should be able to get a response from this request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_api_recvresp_global_settings_get_block(
            datasock, &offset, &status, buffer, &response_size));

    /* status should be successful from the mock. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)status);

    /* the size of the response should be equal to sizeof(uint64_t). */
    ASSERT_EQ(sizeof(uint64_t), response_size);

    /* the response should equal what the mock wrote. */
    uint64_t dummy = 0;
    memcpy(&dummy, buffer, sizeof(uint64_t));
    ASSERT_EQ(321UL, dummy);
}

/**
 * If the global setting set mock callback is not set,
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_global_setting_set)
{
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_VAL[5] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    const size_t EXPECTED_VAL_SIZE = sizeof(EXPECTED_VAL);

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send the global settings get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_api_sendreq_global_settings_set_block(
            datasock, 0U, 0U, EXPECTED_VAL, EXPECTED_VAL_SIZE));

    /* we should be able to get a response from this request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_api_recvresp_global_settings_set_block(
            datasock, &offset, &status));

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the sent global setting set request.
 */
TEST_F(mock_dataservice_test, matches_global_setting_set)
{
    uint32_t child_context = 17U;
    uint64_t key = 93880U;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_VAL[5] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    const size_t EXPECTED_VAL_SIZE = sizeof(EXPECTED_VAL);

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send the global settings get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_api_sendreq_global_settings_set_block(
            datasock, child_context, key, EXPECTED_VAL, EXPECTED_VAL_SIZE));

    /* we should be able to get a response from this request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_api_recvresp_global_settings_set_block(
            datasock, &offset, &status));

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_global_setting_set(
            child_context, key, EXPECTED_VAL_SIZE, EXPECTED_VAL));
}

/**
 * We can override the global setting set call to return an arbitrary value.
 */
TEST_F(mock_dataservice_test, global_setting_set_override)
{
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_VAL[5] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    const size_t EXPECTED_VAL_SIZE = sizeof(EXPECTED_VAL);

    /* set the mock to write a default value. */
    mock->register_callback_global_setting_set(
        [](const dataservice_request_global_setting_set_t&, ostream&) {
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send the global settings get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_api_sendreq_global_settings_set_block(
            datasock, 0U, 0U, EXPECTED_VAL, EXPECTED_VAL_SIZE));

    /* we should be able to get a response from this request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_api_recvresp_global_settings_set_block(
            datasock, &offset, &status));

    /* the mock should return a success status. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
}

/**
 * If the transaction drop mock is not set, then
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_transaction_drop)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x3a, 0x38, 0x9f, 0x37, 0x39, 0xf0, 0x41, 0x28,
        0xbd, 0x31, 0x01, 0xfa, 0xca, 0x83, 0xdb, 0xae
    };

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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
                        &nonblockdatasock, child_context, EXPECTED_TXN_ID);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the sent transaction drop request.
 */
TEST_F(mock_dataservice_test, matches_transaction_drop)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x3a, 0x38, 0x9f, 0x37, 0x39, 0xf0, 0x41, 0x28,
        0xbd, 0x31, 0x01, 0xfa, 0xca, 0x83, 0xdb, 0xae
    };

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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
                        &nonblockdatasock, child_context, EXPECTED_TXN_ID);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_transaction_drop(
            child_context, EXPECTED_TXN_ID));
}

/**
 * If the transaction drop mock is set, then
 * the status code and data it returns is returned in the api call.
 */
TEST_F(mock_dataservice_test, transaction_drop_override)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x3a, 0x38, 0x9f, 0x37, 0x39, 0xf0, 0x41, 0x28,
        0xbd, 0x31, 0x01, 0xfa, 0xca, 0x83, 0xdb, 0xae
    };

    /* mock the transaction drop api call. */
    mock->register_callback_transaction_drop(
        [&](const dataservice_request_transaction_drop_t&,
            std::ostream&) {
            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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
                        &nonblockdatasock, child_context, EXPECTED_TXN_ID);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the mock returns success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
}

/**
 * If the transaction get mock is not set,
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_transaction_get)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x82, 0xfd, 0xa8, 0xd1, 0x6e, 0x45, 0x4e, 0xbf,
        0xae, 0x32, 0xc9, 0xf0, 0x8a, 0x4b, 0x0a, 0xeb
    };
    data_transaction_node_t txn_node;
    void* data = nullptr;
    size_t data_size = 0U;

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get(
                        &nonblockdatasock, &offset, &status, &txn_node,
                        &data, &data_size);

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
                        &nonblockdatasock, child_context, EXPECTED_TXN_ID);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the sent transaction get request.
 */
TEST_F(mock_dataservice_test, matches_transaction_get)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x82, 0xfd, 0xa8, 0xd1, 0x6e, 0x45, 0x4e, 0xbf,
        0xae, 0x32, 0xc9, 0xf0, 0x8a, 0x4b, 0x0a, 0xeb
    };
    data_transaction_node_t txn_node;
    void* data = nullptr;
    size_t data_size = 0U;

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get(
                        &nonblockdatasock, &offset, &status, &txn_node,
                        &data, &data_size);

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
                        &nonblockdatasock, child_context, EXPECTED_TXN_ID);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_transaction_get(
            child_context, EXPECTED_TXN_ID));
}

/**
 * If the transaction get mock is set, then
 * the status code and data it returns is returned in the api call.
 */
TEST_F(mock_dataservice_test, transaction_get_override)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x82, 0xfd, 0xa8, 0xd1, 0x6e, 0x45, 0x4e, 0xbf,
        0xae, 0x32, 0xc9, 0xf0, 0x8a, 0x4b, 0x0a, 0xeb
    };
    const uint8_t EXPECTED_PREV_ID[16] = {
        0x99, 0x57, 0xff, 0x00, 0xc2, 0xf9, 0x4a, 0x79,
        0x8a, 0xac, 0x76, 0x0a, 0x01, 0xe7, 0xd2, 0xd2
    };
    const uint8_t EXPECTED_NEXT_ID[16] = {
        0xc4, 0xb1, 0xc7, 0xe2, 0xe8, 0x94, 0x4a, 0x0f,
        0x82, 0xac, 0x6c, 0x21, 0xdc, 0xc7, 0x77, 0x08
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xad, 0x61, 0x60, 0xc3, 0x6f, 0x36, 0x45, 0x9a,
        0xb2, 0x28, 0xb4, 0xeb, 0x0a, 0x3b, 0xc7, 0x13
    };
    const uint8_t EXPECTED_CERT[5] = { 0x05, 0x04, 0x03, 0x02, 0x01 };
    const size_t EXPECTED_CERT_SIZE = sizeof(EXPECTED_CERT);
    data_transaction_node_t txn_node;
    void* data = nullptr;
    size_t data_size = 0U;

    /* mock the canonized transaction read api call. */
    mock->register_callback_transaction_get(
        [&](const dataservice_request_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_PREV_ID,
                    EXPECTED_NEXT_ID, EXPECTED_ARTIFACT_ID,
                    EXPECTED_CERT, EXPECTED_CERT_SIZE);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get(
                        &nonblockdatasock, &offset, &status, &txn_node,
                        &data, &data_size);

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
                        &nonblockdatasock, child_context, EXPECTED_TXN_ID);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the mock returns success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the node data is correct. */
    EXPECT_EQ(0, memcmp(EXPECTED_TXN_ID, txn_node.key, 16));
    EXPECT_EQ(0, memcmp(EXPECTED_PREV_ID, txn_node.prev, 16));
    EXPECT_EQ(0, memcmp(EXPECTED_NEXT_ID, txn_node.next, 16));
    EXPECT_EQ(0, memcmp(EXPECTED_ARTIFACT_ID, txn_node.artifact_id, 16));
    EXPECT_EQ((int64_t)EXPECTED_CERT_SIZE, ntohll(txn_node.net_txn_cert_size));
    ASSERT_EQ(EXPECTED_CERT_SIZE, data_size);
    ASSERT_NE(nullptr, data);
    EXPECT_EQ(0, memcmp(EXPECTED_CERT, data, data_size));
}

/**
 * If the transaction get first mock is not set, then
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_transaction_get_first)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    data_transaction_node_t txn_node;
    void* data = nullptr;
    size_t data_size = 0U;

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get_first(
                        &nonblockdatasock, &offset, &status, &txn_node,
                        &data, &data_size);

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

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the sent transaction get first request.
 */
TEST_F(mock_dataservice_test, matches_transaction_get_first)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    data_transaction_node_t txn_node;
    void* data = nullptr;
    size_t data_size = 0U;

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get_first(
                        &nonblockdatasock, &offset, &status, &txn_node,
                        &data, &data_size);

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

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_transaction_get_first(
            child_context));
}

/**
 * If the transaction get first mock is set, then
 * the status code and data it returns is returned in the api call.
 */
TEST_F(mock_dataservice_test, transaction_get_first_override)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x82, 0xfd, 0xa8, 0xd1, 0x6e, 0x45, 0x4e, 0xbf,
        0xae, 0x32, 0xc9, 0xf0, 0x8a, 0x4b, 0x0a, 0xeb
    };
    const uint8_t EXPECTED_PREV_ID[16] = {
        0x99, 0x57, 0xff, 0x00, 0xc2, 0xf9, 0x4a, 0x79,
        0x8a, 0xac, 0x76, 0x0a, 0x01, 0xe7, 0xd2, 0xd2
    };
    const uint8_t EXPECTED_NEXT_ID[16] = {
        0xc4, 0xb1, 0xc7, 0xe2, 0xe8, 0x94, 0x4a, 0x0f,
        0x82, 0xac, 0x6c, 0x21, 0xdc, 0xc7, 0x77, 0x08
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xad, 0x61, 0x60, 0xc3, 0x6f, 0x36, 0x45, 0x9a,
        0xb2, 0x28, 0xb4, 0xeb, 0x0a, 0x3b, 0xc7, 0x13
    };
    const uint8_t EXPECTED_CERT[5] = { 0x05, 0x04, 0x03, 0x02, 0x01 };
    const size_t EXPECTED_CERT_SIZE = sizeof(EXPECTED_CERT);
    data_transaction_node_t txn_node;
    void* data = nullptr;
    size_t data_size = 0U;

    /* mock the canonized transaction read api call. */
    mock->register_callback_transaction_get_first(
        [&](const dataservice_request_transaction_get_first_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_transaction_get_first(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_PREV_ID,
                    EXPECTED_NEXT_ID, EXPECTED_ARTIFACT_ID,
                    EXPECTED_CERT, EXPECTED_CERT_SIZE);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    nonblockmode(
        /* onRead. */
        [&]() {
            if (recvresp_status == AGENTD_ERROR_IPC_WOULD_BLOCK)
            {
                recvresp_status =
                    dataservice_api_recvresp_transaction_get_first(
                        &nonblockdatasock, &offset, &status, &txn_node,
                        &data, &data_size);

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

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the mock returns success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the node data is correct. */
    EXPECT_EQ(0, memcmp(EXPECTED_TXN_ID, txn_node.key, 16));
    EXPECT_EQ(0, memcmp(EXPECTED_PREV_ID, txn_node.prev, 16));
    EXPECT_EQ(0, memcmp(EXPECTED_NEXT_ID, txn_node.next, 16));
    EXPECT_EQ(0, memcmp(EXPECTED_ARTIFACT_ID, txn_node.artifact_id, 16));
    EXPECT_EQ((int64_t)EXPECTED_CERT_SIZE, ntohll(txn_node.net_txn_cert_size));
    ASSERT_EQ(EXPECTED_CERT_SIZE, data_size);
    ASSERT_NE(nullptr, data);
    EXPECT_EQ(0, memcmp(EXPECTED_CERT, data, data_size));
}

/**
 * If the transaction submit mock is not set, then
 * the AGENTD_ERROR_DATASERVICE_NOT_FOUND status is returned.
 */
TEST_F(mock_dataservice_test, default_transaction_submit)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x33, 0x0c, 0xf2, 0xb2, 0xcc, 0xec, 0x48, 0xf3,
        0xb9, 0xb6, 0x55, 0xa5, 0xa6, 0x71, 0xfa, 0xa6
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xc7, 0x0a, 0x05, 0x2d, 0x38, 0x3e, 0x4d, 0xe2,
        0x88, 0x18, 0x05, 0x7f, 0x52, 0x8a, 0xfc, 0xd3
    };
    const uint8_t EXPECTED_TXN_CERT[4] = { 0x03, 0x02, 0x01, 0x00 };
    const size_t EXPECTED_TXN_CERT_SIZE = sizeof(EXPECTED_TXN_CERT);

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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
                        &nonblockdatasock, child_context, EXPECTED_TXN_ID,
                        EXPECTED_ARTIFACT_ID,
                        EXPECTED_TXN_CERT, EXPECTED_TXN_CERT_SIZE);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
}

/**
 * Test that we can match against the sent transaction submit request.
 */
TEST_F(mock_dataservice_test, matches_transaction_submit)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x33, 0x0c, 0xf2, 0xb2, 0xcc, 0xec, 0x48, 0xf3,
        0xb9, 0xb6, 0x55, 0xa5, 0xa6, 0x71, 0xfa, 0xa6
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xc7, 0x0a, 0x05, 0x2d, 0x38, 0x3e, 0x4d, 0xe2,
        0x88, 0x18, 0x05, 0x7f, 0x52, 0x8a, 0xfc, 0xd3
    };
    const uint8_t EXPECTED_TXN_CERT[4] = { 0x03, 0x02, 0x01, 0x00 };
    const size_t EXPECTED_TXN_CERT_SIZE = sizeof(EXPECTED_TXN_CERT);

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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
                        &nonblockdatasock, child_context, EXPECTED_TXN_ID,
                        EXPECTED_ARTIFACT_ID,
                        EXPECTED_TXN_CERT, EXPECTED_TXN_CERT_SIZE);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the status code for an empty mock should be
     * AGENTD_ERROR_DATASERVICE_NOT_FOUND. */
    EXPECT_EQ(AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);

    /* stop the mock to ensure that the remote test logging socket is closed. */
    mock->stop();

    /* we can match the request we sent. */
    EXPECT_TRUE(
        mock->request_matches_transaction_submit(
            child_context, EXPECTED_TXN_ID, EXPECTED_ARTIFACT_ID,
            EXPECTED_TXN_CERT_SIZE, EXPECTED_TXN_CERT));
}

/**
 * If the transaction submit mock is set, then
 * the status code and data it returns is returned in the api call.
 */
TEST_F(mock_dataservice_test, transaction_submit_override)
{
    uint32_t child_context = 1023;
    uint32_t offset = 0U;
    uint32_t status = 0U;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x33, 0x0c, 0xf2, 0xb2, 0xcc, 0xec, 0x48, 0xf3,
        0xb9, 0xb6, 0x55, 0xa5, 0xa6, 0x71, 0xfa, 0xa6
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xc7, 0x0a, 0x05, 0x2d, 0x38, 0x3e, 0x4d, 0xe2,
        0x88, 0x18, 0x05, 0x7f, 0x52, 0x8a, 0xfc, 0xd3
    };
    const uint8_t EXPECTED_TXN_CERT[4] = { 0x03, 0x02, 0x01, 0x00 };
    const size_t EXPECTED_TXN_CERT_SIZE = sizeof(EXPECTED_TXN_CERT);

    /* mock the transaction submit api call. */
    mock->register_callback_transaction_submit(
        [&](const dataservice_request_transaction_submit_t&,
            std::ostream&) {
            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock dataservice. */
    mock->start();

    /* we should be able to send and receive the request / resp */
    int sendreq_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
    int recvresp_status = AGENTD_ERROR_IPC_WOULD_BLOCK;
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
                        &nonblockdatasock, child_context, EXPECTED_TXN_ID,
                        EXPECTED_ARTIFACT_ID,
                        EXPECTED_TXN_CERT, EXPECTED_TXN_CERT_SIZE);
            }
        });

    ASSERT_EQ(AGENTD_STATUS_SUCCESS, sendreq_status);
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, recvresp_status);

    /* the mock returns success. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
}
