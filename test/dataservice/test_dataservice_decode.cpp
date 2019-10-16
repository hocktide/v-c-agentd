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
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_root_context_init_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_root_context_init_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_root_context_init(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_root_context_init_decoded)
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

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_root_context_reduce_caps_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_root_context_reduce_caps_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_root_context_reduce_caps(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_root_context_reduce_caps(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_root_context_reduce_caps(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_root_context_reduce_caps_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_root_context_reduce_caps_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_root_context_reduce_caps(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_root_context_reduce_caps(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_root_context_reduce_caps_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_root_context_reduce_caps_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_root_context_reduce_caps(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_root_context_reduce_caps_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x01,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_root_context_reduce_caps_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_root_context_reduce_caps(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_REDUCE_CAPS,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_child_context_create_bad_sizes)
{
    uint8_t resp[100] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x02,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x00000000 */
        0x00, 0x00, 0x00, 0x00
    };
    dataservice_response_child_context_create_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_child_context_create(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_DATA_PACKET_SIZE,
        dataservice_decode_response_child_context_create(
            resp, 3 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_child_context_create_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_child_context_create_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_child_context_create(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_child_context_create(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_child_context_create_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_child_context_create_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_child_context_create(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_child_context_create_decoded)
{
    uint8_t resp[16] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x02,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x00000000 */
        0x00, 0x00, 0x00, 0x00,

        /* child index == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_child_context_create_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_child_context_create(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status code is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the child index is correct. */
    ASSERT_EQ(0x12345678U, dresp.child);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
}
