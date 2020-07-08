/**
 * \file test_unauthorized_protocol_service_isolation.cpp
 *
 * Isolation tests for the unauthorized protocol service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/protocolservice/api.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vpr/disposable.h>

#include "test_unauthorized_protocol_service_isolation.h"

using namespace std;

/**
 * Test that we can spawn the unauthorized protocol service.
 */
TEST_F(unauthorized_protocol_service_isolation_test, simple_spawn)
{
    ASSERT_EQ(0, proto_proc_status);
}

/**
 * Test that writing a bad packet type results in an error.
 */
TEST_F(unauthorized_protocol_service_isolation_test, handshake_request_bad)
{
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    ASSERT_EQ(0, ipc_write_int8_block(protosock, 17));

    /* An invalid packet ends the connection before we can read a valid
     * response. */
    ASSERT_NE(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
}

/**
 * Test that writing a malformed data packet results in an error.
 */
TEST_F(unauthorized_protocol_service_isolation_test, handshake_req_bad_size)
{
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    ASSERT_EQ(0, ipc_write_data_block(protosock, "123", 3));

    /* we return a truncated error response. */
    ASSERT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    EXPECT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
}

/**
 * Test that writing a request id other than one that initiates the
 * handshake results in an error.
 */
TEST_F(unauthorized_protocol_service_isolation_test, handshake_req_bad_reqid)
{
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    uint32_t bad_request_id = htonl(0x01U);
    uint32_t request_offset = htonl(0x00U);
    uint32_t protocol_version_requested = htonl(0x01U);
    uint32_t crypto_suite_version_requested = htonl(VCCRYPT_SUITE_VELO_V1);
    uint8_t entity_uuid[16];
    memset(entity_uuid, 0, sizeof(entity_uuid));

    uint8_t payload[96];
    uint8_t* breq = payload;

    memcpy(breq, &bad_request_id, sizeof(bad_request_id));
    breq += sizeof(bad_request_id);
    memcpy(breq, &request_offset, sizeof(request_offset));
    breq += sizeof(request_offset);
    memcpy(breq, &protocol_version_requested,
        sizeof(protocol_version_requested));
    breq += sizeof(protocol_version_requested);
    memcpy(breq, &crypto_suite_version_requested,
        sizeof(crypto_suite_version_requested));
    breq += sizeof(crypto_suite_version_requested);
    memcpy(breq, entity_uuid, sizeof(entity_uuid));
    breq += sizeof(entity_uuid);
    memcpy(breq, client_key_nonce.data, client_key_nonce.size);
    breq += client_key_nonce.size;
    memcpy(breq, client_challenge_nonce.data, client_challenge_nonce.size);
    breq += client_challenge_nonce.size;

    ASSERT_EQ(0, ipc_write_data_block(protosock, payload, sizeof(payload)));

    /* we return a truncated error response. */
    ASSERT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    EXPECT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
}

/**
 * Test that writing a non-zero offset for the handshake request results in an
 * error.
 */
TEST_F(unauthorized_protocol_service_isolation_test, handshake_req_bad_offset)
{
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    uint32_t request_id = htonl(0x00U);
    uint32_t bad_request_offset = htonl(0x01U);
    uint32_t protocol_version_requested = htonl(0x01U);
    uint32_t crypto_suite_version_requested = htonl(VCCRYPT_SUITE_VELO_V1);
    uint8_t entity_uuid[16];
    memset(entity_uuid, 0, sizeof(entity_uuid));

    uint8_t payload[96];
    uint8_t* breq = payload;

    memcpy(breq, &request_id, sizeof(request_id));
    breq += sizeof(request_id);
    memcpy(breq, &bad_request_offset, sizeof(bad_request_offset));
    breq += sizeof(bad_request_offset);
    memcpy(breq, &protocol_version_requested,
        sizeof(protocol_version_requested));
    breq += sizeof(protocol_version_requested);
    memcpy(breq, &crypto_suite_version_requested,
        sizeof(crypto_suite_version_requested));
    breq += sizeof(crypto_suite_version_requested);
    memcpy(breq, entity_uuid, sizeof(entity_uuid));
    breq += sizeof(entity_uuid);
    memcpy(breq, client_key_nonce.data, client_key_nonce.size);
    breq += client_key_nonce.size;
    memcpy(breq, client_challenge_nonce.data, client_challenge_nonce.size);
    breq += client_challenge_nonce.size;

    ASSERT_EQ(0, ipc_write_data_block(protosock, payload, sizeof(payload)));

    /* we return a truncated error response. */
    ASSERT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    EXPECT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST, (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
}

/**
 * Test that writing a handshake request with a bad entity id results in an
 * error.
 */
TEST_F(unauthorized_protocol_service_isolation_test, handshake_req_bad_entity)
{
    uint32_t offset, status;

    vccrypt_buffer_t server_id;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_challenge_nonce;
    vccrypt_buffer_t shared_secret;

    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_key_nonce));
    memset(client_key_nonce.data, 0, client_key_nonce.size);
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_suite_buffer_init_for_cipher_key_agreement_nonce(
            &suite, &client_challenge_nonce));
    memset(client_challenge_nonce.data, 0, client_challenge_nonce.size);

    uint32_t request_id = htonl(0x00U);
    uint32_t request_offset = htonl(0x00U);
    uint32_t protocol_version_requested = htonl(0x01);
    uint32_t crypto_suite_version_requested = htonl(VCCRYPT_SUITE_VELO_V1);
    uint8_t entity_uuid[16];
    memset(entity_uuid, 0, sizeof(entity_uuid));

    uint8_t payload[96];
    uint8_t* breq = payload;

    memcpy(breq, &request_id, sizeof(request_id));
    breq += sizeof(request_id);
    memcpy(breq, &request_offset, sizeof(request_offset));
    breq += sizeof(request_offset);
    memcpy(breq, &protocol_version_requested,
        sizeof(protocol_version_requested));
    breq += sizeof(protocol_version_requested);
    memcpy(breq, &crypto_suite_version_requested,
        sizeof(crypto_suite_version_requested));
    breq += sizeof(crypto_suite_version_requested);
    memcpy(breq, entity_uuid, sizeof(entity_uuid));
    breq += sizeof(entity_uuid);
    memcpy(breq, client_key_nonce.data, client_key_nonce.size);
    breq += client_key_nonce.size;
    memcpy(breq, client_challenge_nonce.data, client_challenge_nonce.size);
    breq += client_challenge_nonce.size;

    ASSERT_EQ(0, ipc_write_data_block(protosock, payload, sizeof(payload)));

    /* we return an unauthorized error response. */
    ASSERT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED */
    EXPECT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_UNAUTHORIZED, (int)status);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
}

/**
 * Test that writing a valid handshake request results in a valid handshake
 * response.
 */
TEST_F(unauthorized_protocol_service_isolation_test, handshake_request_happy)
{
    uint32_t offset, status;

    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t server_id;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t server_challenge_nonce;

    /* we must have a valid crypto suite for this to work. */
    ASSERT_TRUE(suite_initialized);

    /* write the handshake request to the socket. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_handshake_request_block(
            protosock, &suite, authorized_entity_id, &client_key_nonce,
            &client_challenge_nonce));

    /* This should return successfully. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_STATUS_SUCCESS. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);

    /* the server id is correct. */
    EXPECT_EQ(16U, server_id.size);
    EXPECT_EQ(0, memcmp(server_id.data, agent_id, server_id.size));

    /* the server public key is correct. */
    EXPECT_EQ(32U, server_public_key.size);
    EXPECT_EQ(0,
        memcmp(server_public_key.data, agent_pubkey, server_public_key.size));

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
    dispose((disposable_t*)&server_public_key);
    dispose((disposable_t*)&server_id);
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&server_challenge_nonce);
}

/**
 * Test that writing an unencrypted packet after a valid handshake response
 * causes an error.
 */
TEST_F(unauthorized_protocol_service_isolation_test,
    handshake_response_plaintext_error)
{
    uint32_t offset, status;

    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t server_id;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t server_challenge_nonce;

    /* we must have a valid crypto suite for this to work. */
    ASSERT_TRUE(suite_initialized);

    /* write the handshake request to the socket. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_handshake_request_block(
            protosock, &suite, authorized_entity_id, &client_key_nonce,
            &client_challenge_nonce));

    /* This should return successfully. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_STATUS_SUCCESS. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);

    /* write a garbage packet. */
    ASSERT_EQ(
        54,
        write(
            protosock, "test12345678901234567890123456789012345678901234567890",
            54));

    /* we'll get back an encrypted error response. */
    uint8_t* val;
    uint32_t size;
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        ipc_read_authed_data_block(
            protosock, 0x8000000000000001UL, (void**)&val, &size,
            &suite, &shared_secret));

    /* the value should not be NULL. */
    ASSERT_NE(nullptr, val);
    /* the size of the payload should be 12 bytes. */
    ASSERT_EQ(12U, size);

    /* create a response array for convenience. */
    uint32_t* resparr = (uint32_t*)val;

    /* the request ID should be 0, as the request was malformed. */
    EXPECT_EQ(0U, resparr[0]);
    /* the status code is AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST */
    EXPECT_EQ(AGENTD_ERROR_PROTOCOLSERVICE_MALFORMED_REQUEST,
        (int)ntohl(resparr[1]));
    /* the offset is 0. */
    EXPECT_EQ(0U, resparr[2]);

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
    dispose((disposable_t*)&server_public_key);
    dispose((disposable_t*)&server_id);
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&server_challenge_nonce);
    free(val);
}

/**
 * Test that writing a valid response to the server challenge results in a
 * successful response packet.
 */
TEST_F(unauthorized_protocol_service_isolation_test,
    handshake_response_happy_path)
{
    uint32_t offset, status;

    vccrypt_buffer_t client_key_nonce;
    vccrypt_buffer_t client_challenge_nonce;
    vccrypt_buffer_t server_public_key;
    vccrypt_buffer_t server_id;
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t server_challenge_nonce;

    /* we must have a valid crypto suite for this to work. */
    ASSERT_TRUE(suite_initialized);

    /* write the handshake request to the socket. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_handshake_request_block(
            protosock, &suite, authorized_entity_id, &client_key_nonce,
            &client_challenge_nonce));

    /* This should return successfully. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_handshake_request_block(
            protosock, &suite, &server_id, &client_private_key,
            &server_public_key, &client_key_nonce,
            &client_challenge_nonce, &server_challenge_nonce,
            &shared_secret, &offset, &status));

    /* the offset is always 0 for a handshake response. */
    EXPECT_EQ(0U, offset);

    /* the status code is AGENTD_STATUS_SUCCESS. */
    EXPECT_EQ(AGENTD_STATUS_SUCCESS, (int)status);

    /* send the handshake ack request. */
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_handshake_ack_block(
            protosock, &suite, &client_iv, &shared_secret,
            &server_challenge_nonce));

    /* receive the handshake ack response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_handshake_ack_block(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status));

    /* the status should indicate success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* at this point, we have successfully established a secure channel. */

    dispose((disposable_t*)&client_key_nonce);
    dispose((disposable_t*)&client_challenge_nonce);
    dispose((disposable_t*)&server_public_key);
    dispose((disposable_t*)&server_id);
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&server_challenge_nonce);
}

/**
 * Test that a request to get the latest block ID returns the latest block ID.
 */
TEST_F(unauthorized_protocol_service_isolation_test,
    get_latest_block_id_happy_path)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xb2, 0xf3, 0xfa, 0x16, 0x75, 0x9f, 0x4d, 0x4a,
        0xaf, 0x6b, 0xf7, 0x68, 0x14, 0x35, 0x7d, 0x21
    };
    vccrypt_buffer_t shared_secret;

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the latest block id api call. */
    dataservice->register_callback_block_id_latest_read(
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

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_latest_block_id_get_block(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the response. */
    vccrypt_buffer_t block_id;
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_latest_block_id_get_block(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, &block_id));

    /* the status should indicate success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);
    /* the block_id size should be the correct size. */
    ASSERT_EQ(block_id.size, sizeof(EXPECTED_BLOCK_ID));
    /* the block id should match. */
    ASSERT_EQ(0, memcmp(block_id.data, EXPECTED_BLOCK_ID, block_id.size));

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a latest block_id call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_block_id_latest_read(
            EXPECTED_CHILD_INDEX));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&block_id);
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test that a request to get a block id by height returns that block id.
 */
TEST_F(unauthorized_protocol_service_isolation_test,
    get_block_id_by_height_happy_path)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0x3d, 0x30, 0x6b, 0x0b, 0x73, 0x1d, 0x4b, 0xe9,
        0x84, 0xda, 0x2a, 0xb8, 0xd7, 0x8f, 0x52, 0x30
    };
    const uint64_t EXPECTED_HEIGHT = 117;
    vccrypt_buffer_t shared_secret;

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the latest block id api call. */
    dataservice->register_callback_block_id_by_height_read(
        [&](const dataservice_request_block_id_by_height_read_t& req,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            if (req.block_height != EXPECTED_HEIGHT)
                return AGENTD_ERROR_DATASERVICE_NOT_FOUND;

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

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_block_id_by_height_get_block(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_HEIGHT));

    /* get the response. */
    vccrypt_buffer_t block_id;
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_block_id_by_height_get_block(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, &block_id));

    /* the status should indicate success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);
    /* the block_id size should be the correct size. */
    ASSERT_EQ(block_id.size, sizeof(EXPECTED_BLOCK_ID));
    /* the block id should match. */
    ASSERT_EQ(0, memcmp(block_id.data, EXPECTED_BLOCK_ID, block_id.size));

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a latest block_id call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_block_id_by_height_read(
            EXPECTED_CHILD_INDEX, EXPECTED_HEIGHT));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&block_id);
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test that a request to submit a transaction that is too large fails with an
 * AGENTD_ERROR_PROTOCOLSERVICE_TRANSACTION_VERIFICATION.
 */
TEST_F(unauthorized_protocol_service_isolation_test,
    transaction_submit_big_certificate)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TRANSACTION_ID[16] = {
        0x64, 0x91, 0xf1, 0xcf, 0x34, 0xbb, 0x42, 0x15,
        0x9b, 0xc5, 0x49, 0x1e, 0x7a, 0x46, 0xcd, 0x69
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xc0, 0x9d, 0x7a, 0xed, 0x7a, 0xef, 0x4b, 0x15,
        0x9a, 0xdd, 0xd2, 0x03, 0x59, 0xbc, 0xc8, 0x3a
    };
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t cert;

    /* create the certificate buffer. */
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_buffer_init(&cert, &alloc_opts, 32768));
    memset(cert.data, 0xFE, cert.size);

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the submission request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_transaction_submit(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_TRANSACTION_ID, EXPECTED_ARTIFACT_ID,
            &cert));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_transaction_submit(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status));

    /* the status should indicate failure. */
    ASSERT_EQ(
        AGENTD_ERROR_PROTOCOLSERVICE_TRANSACTION_VERIFICATION, (int)status);

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&cert);
}

/**
 * Test that a request to submit a transaction goes through our mock.
 */
TEST_F(unauthorized_protocol_service_isolation_test,
    transaction_submit_happy_path)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TRANSACTION_ID[16] = {
        0x64, 0x91, 0xf1, 0xcf, 0x34, 0xbb, 0x42, 0x15,
        0x9b, 0xc5, 0x49, 0x1e, 0x7a, 0x46, 0xcd, 0x69
    };
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0xc0, 0x9d, 0x7a, 0xed, 0x7a, 0xef, 0x4b, 0x15,
        0x9a, 0xdd, 0xd2, 0x03, 0x59, 0xbc, 0xc8, 0x3a
    };
    vccrypt_buffer_t shared_secret;
    vccrypt_buffer_t cert;

    /* create the certificate buffer. */
    ASSERT_EQ(VCCRYPT_STATUS_SUCCESS,
        vccrypt_buffer_init(&cert, &alloc_opts, 5000));
    memset(cert.data, 0xFE, cert.size);

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the transaction submit api call. */
    dataservice->register_callback_transaction_submit(
        [&](const dataservice_request_transaction_submit_t&,
            std::ostream&) {
            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the submission request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_transaction_submit(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_TRANSACTION_ID, EXPECTED_ARTIFACT_ID,
            &cert));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_transaction_submit(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status));

    /* the status should indicate success. */
    ASSERT_EQ(
        AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a transaction submit call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_transaction_submit(
            EXPECTED_CHILD_INDEX, EXPECTED_TRANSACTION_ID, EXPECTED_ARTIFACT_ID,
            cert.size, (const uint8_t*)cert.data));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
    dispose((disposable_t*)&cert);
}

/**
 * Test that a request to get a block by id passes a failure condition back when
 * the query fails in our data service mock.
 */
TEST_F(unauthorized_protocol_service_isolation_test, block_get_by_id_not_found)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xca, 0x47, 0xa5, 0xbb, 0x39, 0xaa, 0x44, 0xb2,
        0xb1, 0x7b, 0xc0, 0x55, 0x1a, 0x24, 0x90, 0x9c
    };
    vccrypt_buffer_t shared_secret;
    data_block_node_t data_block_node;
    uint8_t* block_cert = nullptr;
    size_t block_cert_size = 0UL;

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream&) {
            /* block not found. */
            return AGENTD_ERROR_DATASERVICE_NOT_FOUND;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_block_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_BLOCK_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_block_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, &data_block_node, &block_cert, &block_cert_size));

    /* the status should indicate that the record wasn't found. */
    ASSERT_EQ(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_block_read(
            EXPECTED_CHILD_INDEX, EXPECTED_BLOCK_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test the happy path of block_get_by_id
 */
TEST_F(unauthorized_protocol_service_isolation_test, block_get_by_id_happy_path)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xca, 0x47, 0xa5, 0xbb, 0x39, 0xaa, 0x44, 0xb2,
        0xb1, 0x7b, 0xc0, 0x55, 0x1a, 0x24, 0x90, 0x9c
    };
    vccrypt_buffer_t shared_secret;
    data_block_node_t data_block_node;
    uint8_t* block_cert = nullptr;
    size_t block_cert_size = 0UL;

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID, EXPECTED_BLOCK_ID,
                    EXPECTED_BLOCK_ID, EXPECTED_BLOCK_ID, 10, true,
                    EXPECTED_BLOCK_ID, sizeof(EXPECTED_BLOCK_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_block_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_BLOCK_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_block_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, &data_block_node, &block_cert, &block_cert_size));

    /* the status should indicate that the record wasn't found. */
    ASSERT_EQ(
        AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* in the mock above, we hack in the block id as the certificate. */
    ASSERT_EQ(0, memcmp(block_cert, EXPECTED_BLOCK_ID, 16));
    ASSERT_EQ(16U, block_cert_size);

    /* clean up memory. */
    free(block_cert);

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_block_read(
            EXPECTED_CHILD_INDEX, EXPECTED_BLOCK_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test the happy path of block_get_next_id.
 */
TEST_F(unauthorized_protocol_service_isolation_test, block_get_next_id)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xca, 0x47, 0xa5, 0xbb, 0x39, 0xaa, 0x44, 0xb2,
        0xb1, 0x7b, 0xc0, 0x55, 0x1a, 0x24, 0x90, 0x9c
    };
    const uint8_t EXPECTED_NEXT_BLOCK_ID[16] = {
        0xbd, 0xbc, 0xbd, 0x4a, 0x2d, 0x39, 0x4f, 0x23,
        0xbc, 0xc6, 0xf7, 0xb8, 0x03, 0xa5, 0x7f, 0x6a
    };
    vccrypt_buffer_t shared_secret;
    uint8_t next_id[16];

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID, EXPECTED_BLOCK_ID,
                    EXPECTED_NEXT_BLOCK_ID, EXPECTED_BLOCK_ID, 10, false,
                    EXPECTED_BLOCK_ID, sizeof(EXPECTED_BLOCK_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_block_next_id_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_BLOCK_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_block_next_id_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, next_id));

    /* the status should indicate success. */
    ASSERT_EQ(
        AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* in the mock above, we hack in the next block id. */
    ASSERT_EQ(0, memcmp(next_id, EXPECTED_NEXT_BLOCK_ID, 16));

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_block_read(
            EXPECTED_CHILD_INDEX, EXPECTED_BLOCK_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test that block_get_next_id returns NOT_FOUND if the block id is the end
 * sentry.
 */
TEST_F(unauthorized_protocol_service_isolation_test, block_get_next_id_end)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xca, 0x47, 0xa5, 0xbb, 0x39, 0xaa, 0x44, 0xb2,
        0xb1, 0x7b, 0xc0, 0x55, 0x1a, 0x24, 0x90, 0x9c
    };
    const uint8_t EXPECTED_NEXT_BLOCK_ID[16] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    vccrypt_buffer_t shared_secret;
    uint8_t next_id[16];

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID, EXPECTED_BLOCK_ID,
                    EXPECTED_NEXT_BLOCK_ID, EXPECTED_BLOCK_ID, 10, false,
                    EXPECTED_BLOCK_ID, sizeof(EXPECTED_BLOCK_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_block_next_id_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_BLOCK_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_block_next_id_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, next_id));

    /* the status should indicate failure. */
    ASSERT_EQ(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_block_read(
            EXPECTED_CHILD_INDEX, EXPECTED_BLOCK_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test the happy path of block_get_prev_id.
 */
TEST_F(unauthorized_protocol_service_isolation_test, block_get_prev_id)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xca, 0x47, 0xa5, 0xbb, 0x39, 0xaa, 0x44, 0xb2,
        0xb1, 0x7b, 0xc0, 0x55, 0x1a, 0x24, 0x90, 0x9c
    };
    const uint8_t EXPECTED_PREV_BLOCK_ID[16] = {
        0x58, 0x73, 0x64, 0xa8, 0x4d, 0x75, 0x41, 0x40,
        0x84, 0x76, 0x9f, 0x4e, 0x12, 0xa4, 0xdb, 0xb0
    };
    vccrypt_buffer_t shared_secret;
    uint8_t prev_id[16];

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID,
                    EXPECTED_PREV_BLOCK_ID, EXPECTED_BLOCK_ID, EXPECTED_BLOCK_ID,
                    10, false, EXPECTED_BLOCK_ID, sizeof(EXPECTED_BLOCK_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_block_prev_id_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_BLOCK_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_block_prev_id_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, prev_id));

    /* the status should indicate success. */
    ASSERT_EQ(
        AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* in the mock above, we hack in the prev block id. */
    ASSERT_EQ(0, memcmp(prev_id, EXPECTED_PREV_BLOCK_ID, 16));

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_block_read(
            EXPECTED_CHILD_INDEX, EXPECTED_BLOCK_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test that block_get_prev_id returns NOT_FOUND if the block id is the begin
 * sentry.
 */
TEST_F(unauthorized_protocol_service_isolation_test, block_get_prev_id_end)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_BLOCK_ID[16] = {
        0xca, 0x47, 0xa5, 0xbb, 0x39, 0xaa, 0x44, 0xb2,
        0xb1, 0x7b, 0xc0, 0x55, 0x1a, 0x24, 0x90, 0x9c
    };
    const uint8_t EXPECTED_PREV_BLOCK_ID[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    vccrypt_buffer_t shared_secret;
    uint8_t prev_id[16];

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_block_read(
        [&](const dataservice_request_block_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_block_read(
                    &payload, &payload_size, EXPECTED_BLOCK_ID,
                    EXPECTED_PREV_BLOCK_ID, EXPECTED_BLOCK_ID, EXPECTED_BLOCK_ID,
                    10, false, EXPECTED_BLOCK_ID, sizeof(EXPECTED_BLOCK_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_block_prev_id_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_BLOCK_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_block_prev_id_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, prev_id));

    /* the status should indicate failure. */
    ASSERT_EQ(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_block_read(
            EXPECTED_CHILD_INDEX, EXPECTED_BLOCK_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test the happy path of transaction_get_by_id
 */
TEST_F(unauthorized_protocol_service_isolation_test, txn_get_by_id_happy_path)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    vccrypt_buffer_t shared_secret;
    data_transaction_node_t data_txn_node;
    uint8_t* txn_cert = nullptr;
    size_t txn_cert_size = 0UL;

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_TXN_ID,
                    EXPECTED_TXN_ID, EXPECTED_TXN_ID, EXPECTED_TXN_ID, 10,
                    true, EXPECTED_TXN_ID, sizeof(EXPECTED_TXN_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_transaction_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_TXN_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_transaction_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, &data_txn_node, &txn_cert, &txn_cert_size));

    /* the status should indicate that the record wasn't found. */
    ASSERT_EQ(
        AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* in the mock above, we hack in the txn id as the certificate. */
    ASSERT_EQ(0, memcmp(txn_cert, EXPECTED_TXN_ID, 16));
    ASSERT_EQ(16U, txn_cert_size);

    /* clean up memory. */
    free(txn_cert);

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_canonized_transaction_get(
            EXPECTED_CHILD_INDEX, EXPECTED_TXN_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test the happy path of transaction_get_next_id.
 */
TEST_F(unauthorized_protocol_service_isolation_test, txn_get_next_id_happy_path)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_NEXT_TXN_ID[16] = {
        0xa8, 0x33, 0x7c, 0x29, 0x26, 0xfa, 0x48, 0x4e,
        0x9f, 0x29, 0x6c, 0xe7, 0xb3, 0x3e, 0x4a, 0x65
    };
    vccrypt_buffer_t shared_secret;
    uint8_t next_id[16];

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_TXN_ID,
                    EXPECTED_NEXT_TXN_ID, EXPECTED_TXN_ID, EXPECTED_TXN_ID, 10,
                    true, EXPECTED_TXN_ID, sizeof(EXPECTED_TXN_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_transaction_next_id_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_TXN_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_transaction_next_id_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, next_id));

    /* the status should indicate that the record wasn't found. */
    ASSERT_EQ(
        AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* we should get the next txn id. */
    ASSERT_EQ(0, memcmp(next_id, EXPECTED_NEXT_TXN_ID, 16));

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_canonized_transaction_get(
            EXPECTED_CHILD_INDEX, EXPECTED_TXN_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test that transaction_get_next_id returns NOT_FOUND if the block id is the
 * end sentry.
 */
TEST_F(unauthorized_protocol_service_isolation_test, txn_get_next_id_end)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_NEXT_TXN_ID[16] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
    vccrypt_buffer_t shared_secret;
    uint8_t next_id[16];

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_TXN_ID,
                    EXPECTED_NEXT_TXN_ID, EXPECTED_TXN_ID, EXPECTED_TXN_ID, 10,
                    true, EXPECTED_TXN_ID, sizeof(EXPECTED_TXN_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_transaction_next_id_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_TXN_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_transaction_next_id_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, next_id));

    /* the status should indicate failure. */
    ASSERT_EQ(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_canonized_transaction_get(
            EXPECTED_CHILD_INDEX, EXPECTED_TXN_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test the happy path of transaction_get_prev_id.
 */
TEST_F(unauthorized_protocol_service_isolation_test, txn_get_prev_id_happy_path)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_PREV_TXN_ID[16] = {
        0x3d, 0x36, 0x93, 0x5c, 0x9d, 0x8d, 0x49, 0xbe,
        0xab, 0x76, 0xbf, 0xf2, 0x62, 0xe8, 0x53, 0x60
    };
    vccrypt_buffer_t shared_secret;
    uint8_t prev_id[16];

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_PREV_TXN_ID,
                    EXPECTED_TXN_ID, EXPECTED_TXN_ID, EXPECTED_TXN_ID, 10,
                    true, EXPECTED_TXN_ID, sizeof(EXPECTED_TXN_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_transaction_prev_id_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_TXN_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_transaction_prev_id_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, prev_id));

    /* the status should indicate that the record wasn't found. */
    ASSERT_EQ(
        AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* we should get the prev txn id. */
    ASSERT_EQ(0, memcmp(prev_id, EXPECTED_PREV_TXN_ID, 16));

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_canonized_transaction_get(
            EXPECTED_CHILD_INDEX, EXPECTED_TXN_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test that transaction_get_prev_id returns NOT_FOUND if the block id is the
 * end sentry.
 */
TEST_F(unauthorized_protocol_service_isolation_test, txn_get_prev_id_end)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_PREV_TXN_ID[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    vccrypt_buffer_t shared_secret;
    uint8_t prev_id[16];

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_PREV_TXN_ID,
                    EXPECTED_TXN_ID, EXPECTED_TXN_ID, EXPECTED_TXN_ID, 10,
                    true, EXPECTED_TXN_ID, sizeof(EXPECTED_TXN_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_transaction_prev_id_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_TXN_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_transaction_prev_id_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, prev_id));

    /* the status should indicate failure. */
    ASSERT_EQ(
        AGENTD_ERROR_DATASERVICE_NOT_FOUND, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_canonized_transaction_get(
            EXPECTED_CHILD_INDEX, EXPECTED_TXN_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test the happy path of transaction_get_block_id.
 */
TEST_F(unauthorized_protocol_service_isolation_test, txn_get_block_id_happy_path)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_TXN_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_BLOCK_TXN_ID[16] = {
        0x18, 0x70, 0xe6, 0x2a, 0xff, 0xf2, 0x44, 0x5c,
        0x90, 0xe0, 0xbd, 0xb0, 0x3c, 0xee, 0xe7, 0x5a
    };
    vccrypt_buffer_t shared_secret;
    uint8_t block_id[16];

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_canonized_transaction_get(
        [&](const dataservice_request_canonized_transaction_get_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_canonized_transaction_get(
                    &payload, &payload_size, EXPECTED_TXN_ID, EXPECTED_TXN_ID,
                    EXPECTED_TXN_ID, EXPECTED_TXN_ID, EXPECTED_BLOCK_TXN_ID, 10,
                    true, EXPECTED_TXN_ID, sizeof(EXPECTED_TXN_ID));
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_transaction_block_id_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_TXN_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_transaction_block_id_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, block_id));

    /* the status should indicate that the record wasn't found. */
    ASSERT_EQ(
        AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* we should get the block txn id. */
    ASSERT_EQ(0, memcmp(block_id, EXPECTED_BLOCK_TXN_ID, 16));

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_canonized_transaction_get(
            EXPECTED_CHILD_INDEX, EXPECTED_TXN_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test the happy path of artifact_get_first_txn_id.
 */
TEST_F(unauthorized_protocol_service_isolation_test, artifact_first_txn_happy)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_FIRST_TXN_ID[16] = {
        0x18, 0x70, 0xe6, 0x2a, 0xff, 0xf2, 0x44, 0x5c,
        0x90, 0xe0, 0xbd, 0xb0, 0x3c, 0xee, 0xe7, 0x5a
    };
    vccrypt_buffer_t shared_secret;
    uint8_t first_txn_id[16];

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_payload_artifact_read(
        [&](const dataservice_request_payload_artifact_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_payload_artifact_read(
                    &payload, &payload_size, EXPECTED_ARTIFACT_ID,
                    EXPECTED_FIRST_TXN_ID, zero_uuid, 10, 12, 77);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_artifact_first_txn_id_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_ARTIFACT_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_artifact_first_txn_id_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, first_txn_id));

    /* the status should indicate that the record wasn't found. */
    ASSERT_EQ(
        AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* we should get the first txn id. */
    ASSERT_EQ(0, memcmp(first_txn_id, EXPECTED_FIRST_TXN_ID, 16));

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_payload_artifact_read(
            EXPECTED_CHILD_INDEX, EXPECTED_ARTIFACT_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test the happy path of artifact_get_last_txn_id.
 */
TEST_F(unauthorized_protocol_service_isolation_test, artifact_last_txn_happy)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    const uint8_t EXPECTED_ARTIFACT_ID[16] = {
        0x97, 0xd0, 0x56, 0x30, 0xbb, 0xad, 0x4c, 0xee,
        0x8f, 0x97, 0x32, 0x98, 0x13, 0x0b, 0xbe, 0x3d
    };
    const uint8_t EXPECTED_LAST_TXN_ID[16] = {
        0x18, 0x70, 0xe6, 0x2a, 0xff, 0xf2, 0x44, 0x5c,
        0x90, 0xe0, 0xbd, 0xb0, 0x3c, 0xee, 0xe7, 0x5a
    };
    vccrypt_buffer_t shared_secret;
    uint8_t last_txn_id[16];

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* mock the block get call. */
    dataservice->register_callback_payload_artifact_read(
        [&](const dataservice_request_payload_artifact_read_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_payload_artifact_read(
                    &payload, &payload_size, EXPECTED_ARTIFACT_ID,
                    zero_uuid, EXPECTED_LAST_TXN_ID, 10, 12, 77);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the block get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_artifact_last_txn_id_get(
            protosock, &suite, &client_iv, &shared_secret,
            EXPECTED_ARTIFACT_ID));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_artifact_last_txn_id_get(
            protosock, &suite, &server_iv, &shared_secret, &offset,
            &status, last_txn_id));

    /* the status should indicate that the record wasn't found. */
    ASSERT_EQ(
        AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* we should get the first txn id. */
    ASSERT_EQ(0, memcmp(last_txn_id, EXPECTED_LAST_TXN_ID, 16));

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_sendreq_close(
            protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
        protocolservice_api_recvresp_close(
            protosock, &suite, &server_iv, &shared_secret));

    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* a block get call should have been made. */
    EXPECT_TRUE(
        dataservice->request_matches_payload_artifact_read(
            EXPECTED_CHILD_INDEX, EXPECTED_ARTIFACT_ID));

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}

/**
 * Test the status api method.
 */
TEST_F(unauthorized_protocol_service_isolation_test, status_happy)
{
    uint32_t offset, status;
    uint64_t client_iv = 0;
    uint64_t server_iv = 0;
    vccrypt_buffer_t shared_secret;

    /* register dataservice helper mocks. */
    ASSERT_EQ(0, dataservice_mock_register_helper());

    /* start the mock. */
    dataservice->start();

    /* do the handshake, populating the shared secret on success. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
              do_handshake(&shared_secret, &server_iv, &client_iv));

    /* send the status get request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
              protocolservice_api_sendreq_status_get(
                    protosock, &suite, &client_iv, &shared_secret));

    /* get the response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
              protocolservice_api_recvresp_status_get(
                    protosock, &suite, &server_iv, &shared_secret, &offset,
                    &status));

    /* the status should indicate success. */
    ASSERT_EQ(
        AGENTD_STATUS_SUCCESS, (int)status);
    /* the offset should be zero. */
    ASSERT_EQ(0U, offset);

    /* send the close request. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
              protocolservice_api_sendreq_close(
                    protosock, &suite, &client_iv, &shared_secret));

    /* get the close response. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS,
              protocolservice_api_recvresp_close(
                    protosock, &suite, &server_iv, &shared_secret));
 
    /* close the socket */
    close(protosock);

    /* stop the mock. */
    dataservice->stop();

    /* verify proper connection setup. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_setup());

    /* verify proper connection teardown. */
    EXPECT_EQ(0, dataservice_mock_valid_connection_teardown());

    /* clean up. */
    dispose((disposable_t*)&shared_secret);
}
