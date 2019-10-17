/**
 * \file test_dataservice_decode.cpp
 *
 * Unit tests for decode methods in dataservice async_api.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
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

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_child_context_close_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_child_context_close_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_child_context_close(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_child_context_close(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_child_context_close(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_child_context_close_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_child_context_close_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_child_context_close(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_child_context_close(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_child_context_close_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_child_context_close_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_child_context_close(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_child_context_close_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x03,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_child_context_close_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_child_context_close(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE,
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
TEST(dataservice_decode_test, response_global_settings_get_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_global_settings_get_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_global_settings_get(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_global_settings_get(
            resp, 2 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_global_settings_get_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_global_settings_get_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_global_settings_get(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_global_settings_get(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_child_global_settings_get_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_global_settings_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_global_settings_get(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_global_settings_get_decoded)
{
    uint8_t resp[15] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x07,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == AGENTD_STATUS_SUCCESS */
        0x00, 0x00, 0x00, 0x00,

        /* global setting data. */
        0x01, 0x02, 0x03
    };
    dataservice_response_global_settings_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_global_settings_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
    /* the data pointer should be set correctly. */
    ASSERT_EQ(resp + 12, dresp.data);
    /* the data_size should be correct. */
    ASSERT_EQ(3U, dresp.data_size);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_global_settings_set_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_global_settings_set_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_global_settings_set(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_global_settings_set(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_global_settings_set(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_global_settings_set_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_global_settings_set_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_global_settings_set(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_global_settings_set(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_global_settings_set_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_global_settings_set_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_global_settings_set(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_global_settings_set_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x08,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_global_settings_set_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_global_settings_set(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE,
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
TEST(dataservice_decode_test, response_transaction_submit_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_submit_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_submit(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_submit(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_submit(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_transaction_submit_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_submit_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_submit(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_submit(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_transaction_submit_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_submit_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_transaction_submit(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_transaction_submit_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x0F,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_submit_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_submit(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT,
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
TEST(dataservice_decode_test, response_transaction_get_first_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_get_first_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_get_first(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_get_first(
            resp, 2 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_transaction_get_first_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_get_first_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_get_first(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_get_first(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_transaction_get_first_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_get_first_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_transaction_get_first(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_transaction_get_first_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x10,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_get_first_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_get_first(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_FIRST_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that a response packet is successfully decoded with a complete payload.
 */
TEST(dataservice_decode_test,
    response_transaction_get_first_decoded_full_payload)
{
    const uint8_t EXPECTED_NODE_KEY[] = {
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef
    };

    const uint8_t EXPECTED_NODE_PREV[] = {
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0
    };

    const uint8_t EXPECTED_NODE_NEXT[] = {
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c
    };

    const uint8_t EXPECTED_NODE_ARTIFACT_ID[] = {
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7
    };

    uint8_t resp[80] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x10,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == AGENTD_STATUS_SUCCESS. */
        0x00, 0x00, 0x00, 0x00,

        /* node.key */
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef,

        /* node.prev */
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0,

        /* node.next */
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c,

        /* node.artifact_id */
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7,

        /* data */
        0x01, 0x02, 0x03, 0x04
    };
    dataservice_response_transaction_get_first_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_get_first(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_FIRST_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
    /* the node key should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_KEY, dresp.node.key, 16));
    /* the node prev should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_PREV, dresp.node.prev, 16));
    /* the node next should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_NEXT, dresp.node.next, 16));
    /* the node artifact_id should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_ARTIFACT_ID, dresp.node.artifact_id, 16));
    /* the node net size should match */
    ASSERT_EQ(dresp.data_size, (size_t)ntohll(dresp.node.net_txn_cert_size));
    /* the data pointer should be correct. */
    ASSERT_EQ(resp + 76, dresp.data);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_transaction_get_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_get_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_get(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_get(
            resp, 2 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_transaction_get_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_get_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_get(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_get(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_transaction_get_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_transaction_get(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_transaction_get_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x11,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that a response packet is successfully decoded with a complete payload.
 */
TEST(dataservice_decode_test,
    response_transaction_get_decoded_full_payload)
{
    const uint8_t EXPECTED_NODE_KEY[] = {
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef
    };

    const uint8_t EXPECTED_NODE_PREV[] = {
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0
    };

    const uint8_t EXPECTED_NODE_NEXT[] = {
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c
    };

    const uint8_t EXPECTED_NODE_ARTIFACT_ID[] = {
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7
    };

    uint8_t resp[80] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x11,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == AGENTD_STATUS_SUCCESS. */
        0x00, 0x00, 0x00, 0x00,

        /* node.key */
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef,

        /* node.prev */
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0,

        /* node.next */
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c,

        /* node.artifact_id */
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7,

        /* data */
        0x01, 0x02, 0x03, 0x04
    };
    dataservice_response_transaction_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
    /* the node key should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_KEY, dresp.node.key, 16));
    /* the node prev should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_PREV, dresp.node.prev, 16));
    /* the node next should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_NEXT, dresp.node.next, 16));
    /* the node artifact_id should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_ARTIFACT_ID, dresp.node.artifact_id, 16));
    /* the node net size should match */
    ASSERT_EQ(dresp.data_size, (size_t)ntohll(dresp.node.net_txn_cert_size));
    /* the data pointer should be correct. */
    ASSERT_EQ(resp + 76, dresp.data);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_canonized_transaction_get_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_canonized_transaction_get_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_canonized_transaction_get(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_canonized_transaction_get(
            resp, 2 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_canonized_transaction_get_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_canonized_transaction_get_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_canonized_transaction_get(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_canonized_transaction_get(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test,
    response_canonized_transaction_get_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_canonized_transaction_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_canonized_transaction_get(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_canonized_transaction_get_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x0E,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_canonized_transaction_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_canonized_transaction_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_TRANSACTION_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}

/**
 * Test that a response packet is successfully decoded with a complete payload.
 */
TEST(dataservice_decode_test,
    response_canonized_transaction_get_decoded_full_payload)
{
    const uint8_t EXPECTED_NODE_KEY[] = {
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef
    };

    const uint8_t EXPECTED_NODE_PREV[] = {
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0
    };

    const uint8_t EXPECTED_NODE_NEXT[] = {
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c
    };

    const uint8_t EXPECTED_NODE_ARTIFACT_ID[] = {
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7
    };

    const uint8_t EXPECTED_NODE_BLOCK_ID[] = {
        0x43, 0x9b, 0xd7, 0xe6, 0xd9, 0xea, 0x43, 0x78,
        0x97, 0x6a, 0xa3, 0x6e, 0x9b, 0x22, 0x0a, 0xbd
    };

    uint8_t resp[96] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x0E,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == AGENTD_STATUS_SUCCESS. */
        0x00, 0x00, 0x00, 0x00,

        /* node.key */
        0x37, 0xfb, 0x38, 0xd3, 0xfe, 0x6b, 0x4e, 0x9c,
        0xba, 0x15, 0x91, 0xbe, 0xf7, 0xf3, 0x87, 0xef,

        /* node.prev */
        0x76, 0xad, 0xbc, 0xb7, 0xbe, 0xdc, 0x45, 0xbe,
        0xa9, 0x52, 0xfa, 0x8c, 0xfa, 0x2f, 0x53, 0xa0,

        /* node.next */
        0xf5, 0x17, 0xda, 0x53, 0xcb, 0x26, 0x45, 0x45,
        0xaa, 0x62, 0x8f, 0x2b, 0x7f, 0x16, 0xfb, 0x7c,

        /* node.artifact_id */
        0xc7, 0xe6, 0x53, 0x0d, 0x84, 0x45, 0x48, 0x58,
        0x82, 0xc1, 0x96, 0x41, 0x7b, 0xe1, 0x89, 0xf7,

        /* node.block_id */
        0x43, 0x9b, 0xd7, 0xe6, 0xd9, 0xea, 0x43, 0x78,
        0x97, 0x6a, 0xa3, 0x6e, 0x9b, 0x22, 0x0a, 0xbd,

        /* data */
        0x01, 0x02, 0x03, 0x04
    };
    dataservice_response_canonized_transaction_get_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_canonized_transaction_get(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_TRANSACTION_READ,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(sizeof(dresp) - sizeof(dresp.hdr), dresp.hdr.payload_size);
    /* the node key should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_KEY, dresp.node.key, 16));
    /* the node prev should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_PREV, dresp.node.prev, 16));
    /* the node next should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_NEXT, dresp.node.next, 16));
    /* the node artifact_id should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_ARTIFACT_ID, dresp.node.artifact_id, 16));
    /* the node block_id should match. */
    ASSERT_EQ(0, memcmp(EXPECTED_NODE_BLOCK_ID, dresp.node.block_id, 16));
    /* the node net size should match */
    ASSERT_EQ(dresp.data_size, (size_t)ntohll(dresp.node.net_txn_cert_size));
    /* the data pointer should be correct. */
    ASSERT_EQ(resp + 92, dresp.data);
}

/**
 * Test that we check for sizes when decoding.
 */
TEST(dataservice_decode_test, response_transaction_drop_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_drop_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_drop(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_drop(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_transaction_drop(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_transaction_drop_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_transaction_drop_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_drop(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_transaction_drop(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_transaction_drop_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_drop_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_transaction_drop(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_transaction_drop_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x12,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_transaction_drop_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_transaction_drop(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_DROP,
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
TEST(dataservice_decode_test, response_block_make_bad_sizes)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_block_make_t dresp;

    /* a zero size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_block_make(
            resp, 0, &dresp));

    /* a truncated size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_block_make(
            resp, 2 * sizeof(uint32_t), &dresp));

    /* a "too large" size is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_PACKET_INVALID_SIZE,
        dataservice_decode_response_block_make(
            resp, 4 * sizeof(uint32_t), &dresp));
}

/**
 * Test that we perform null checks in the decode.
 */
TEST(dataservice_decode_test, response_block_make_null_checks)
{
    uint8_t resp[100] = { 0 };
    dataservice_response_block_make_t dresp;

    /* a null response packet pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_block_make(
            nullptr, 3 * sizeof(uint32_t), &dresp));

    /* a null decoded response structure pointer is invalid. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RESPONSE_INVALID_PARAMETER,
        dataservice_decode_response_block_make(
            resp, 3 * sizeof(uint32_t), nullptr));
}

/**
 * Test that a response packet with an invalid method code returns an error.
 */
TEST(dataservice_decode_test, response_block_make_bad_method_code)
{
    uint8_t resp[12] = {
        /* bad method code. */
        0x80, 0x00, 0x00, 0x00,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_block_make_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_ERROR_DATASERVICE_RECVRESP_UNEXPECTED_METHOD_CODE,
        dataservice_decode_response_block_make(
            resp, sizeof(resp), &dresp));
}

/**
 * Test that a response packet is successfully decoded.
 */
TEST(dataservice_decode_test, response_block_make_decoded)
{
    uint8_t resp[12] = {
        /* method code. */
        0x00, 0x00, 0x00, 0x14,

        /* offset == 1023 */
        0x00, 0x00, 0x03, 0xFF,

        /* status == 0x12345678 */
        0x12, 0x34, 0x56, 0x78
    };
    dataservice_response_block_make_t dresp;

    /* a valid response is successfully decoded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        dataservice_decode_response_block_make(
            resp, sizeof(resp), &dresp));

    /* the disposer is set to the memset disposer. */
    ASSERT_EQ(&dataservice_decode_response_memset_disposer,
        dresp.hdr.hdr.dispose);
    /* the method code is correct. */
    ASSERT_EQ(DATASERVICE_API_METHOD_APP_BLOCK_WRITE,
        dresp.hdr.method_code);
    /* the offset is correct. */
    ASSERT_EQ(1023U, dresp.hdr.offset);
    /* the status is correct. */
    ASSERT_EQ(0x12345678U, dresp.hdr.status);
    /* the payload size is correct. */
    ASSERT_EQ(0U, dresp.hdr.payload_size);
}
