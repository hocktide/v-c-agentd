/**
 * \file test_dataservice_decode.cpp
 *
 * Unit tests for decode methods in dataservice async_api.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <vpr/disposable.h>
#include <gtest/gtest.h>

using namespace std;

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_root_context_init_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_root_context_init_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_root_context_init(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_root_context_init(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_root_context_init(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_root_context_init_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_root_context_init_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_root_context_init(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_root_context_init(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_root_context_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_root_context_init_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_root_context_init(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}
