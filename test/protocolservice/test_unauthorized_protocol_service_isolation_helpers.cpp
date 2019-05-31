/**
 * \file test_unauthorized_protocol_service_isolation_helpers.cpp
 *
 * Helpers for the unauthorized protocol service isolation test.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/protocolservice.h>
#include <agentd/randomservice.h>
#include <vpr/allocator/malloc_allocator.h>

#include "test_unauthorized_protocol_service_isolation.h"

using namespace std;

const uint8_t unauthorized_protocol_service_isolation_test::dir_key[32] = {
    0x7e, 0x4b, 0xb1, 0x5d, 0xb5, 0x00, 0x41, 0x95,
    0xb0, 0xed, 0x43, 0x59, 0x43, 0x20, 0x9b, 0x72,
    0x28, 0x07, 0xad, 0xbb, 0x87, 0x70, 0x49, 0x8a,
    0xac, 0x89, 0x44, 0xcb, 0x23, 0x56, 0x67, 0x3f
};

const uint8_t
    unauthorized_protocol_service_isolation_test::authorized_entity_id[16] = {
        0x6c, 0x36, 0x2b, 0x3e, 0x90, 0x81, 0x4f, 0xcb,
        0x80, 0xfe, 0x16, 0x35, 0x4e, 0x0a, 0xe2, 0x8f
    };

const char*
    unauthorized_protocol_service_isolation_test::authorized_entity_id_string =
        "6c362b3e-9081-4fcb-80fe-16354e0ae28f";

const uint8_t
    unauthorized_protocol_service_isolation_test::authorized_entity_privkey[32] = {
        0x77, 0x07, 0x6d, 0x0a, 0x73, 0x18, 0xa5, 0x7d,
        0x3c, 0x16, 0xc1, 0x72, 0x51, 0xb2, 0x66, 0x45,
        0xdf, 0x4c, 0x2f, 0x87, 0xeb, 0xc0, 0x99, 0x2a,
        0xb1, 0x77, 0xfb, 0xa5, 0x1d, 0xb9, 0x2c, 0x2a
    };

const uint8_t
    unauthorized_protocol_service_isolation_test::authorized_entity_pubkey[32] = {
        0x85, 0x20, 0xf0, 0x09, 0x89, 0x30, 0xa7, 0x54,
        0x74, 0x8b, 0x7d, 0xdc, 0xb4, 0x3e, 0xf7, 0x5a,
        0x0d, 0xbf, 0x3a, 0x0d, 0x26, 0x38, 0x1a, 0xf4,
        0xeb, 0xa4, 0xa9, 0x8e, 0xaa, 0x9b, 0x4e, 0x6a
    };

const char*
    unauthorized_protocol_service_isolation_test::authorized_entity_pubkey_string =
        "8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a";

const uint8_t unauthorized_protocol_service_isolation_test::agent_id[16] = {
    0x3d, 0x96, 0x3f, 0x54, 0x83, 0xe2, 0x4b, 0x0d,
    0x86, 0xa1, 0x81, 0xb6, 0xaa, 0xaa, 0x5c, 0x1b
};

const char* unauthorized_protocol_service_isolation_test::agent_id_string =
    "3d963f54-83e2-4b0d-86a1-81b6aaaa5c1b";

const uint8_t unauthorized_protocol_service_isolation_test::agent_pubkey[32] = {
    0xde, 0x9e, 0xdb, 0x7d, 0x7b, 0x7d, 0xc1, 0xb4,
    0xd3, 0x5b, 0x61, 0xc2, 0xec, 0xe4, 0x35, 0x37,
    0x3f, 0x83, 0x43, 0xc8, 0x5b, 0x78, 0x67, 0x4d,
    0xad, 0xfc, 0x7e, 0x14, 0x6f, 0x88, 0x2b, 0x4f
};

const char* unauthorized_protocol_service_isolation_test::agent_pubkey_string =
    "de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f";

const uint8_t unauthorized_protocol_service_isolation_test::agent_privkey[32] = {
    0x5d, 0xab, 0x08, 0x7e, 0x62, 0x4a, 0x8a, 0x4b,
    0x79, 0xe1, 0x7f, 0x8b, 0x83, 0x80, 0x0e, 0xe6,
    0x6f, 0x3b, 0xb1, 0x29, 0x26, 0x18, 0xb6, 0xfd,
    0x1c, 0x2f, 0x8b, 0x27, 0xff, 0x88, 0xe0, 0xeb
};

const char* unauthorized_protocol_service_isolation_test::agent_privkey_string =
    "5dab087e624a8a4b79e17f8b83800ee66f3bb1292618b6fd1c2f8b27ff88e0eb";

void unauthorized_protocol_service_isolation_test::SetUp()
{
    vccrypt_suite_register_velo_v1();

    /* initialize allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* initialize the crypto suite. */
    if (VCCRYPT_STATUS_SUCCESS ==
        vccrypt_suite_options_init(
            &suite, &alloc_opts, VCCRYPT_SUITE_VELO_V1))
    {
        suite_instance_initialized = true;
    }
    else
    {
        suite_instance_initialized = false;
    }

    /* set up the client private key. */
    if (VCCRYPT_STATUS_SUCCESS == vccrypt_buffer_init(&client_private_key, &alloc_opts, sizeof(authorized_entity_privkey)))
    {
        memcpy(
            client_private_key.data, authorized_entity_privkey,
            client_private_key.size);
        client_private_key_initialized = true;
    }
    else
    {
        client_private_key_initialized = false;
    }

    if (suite_instance_initialized && client_private_key_initialized)
    {
        suite_initialized = true;
    }

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

    /* hard-code some details for testing the agent. */
    setenv("AGENTD_AUTHORIZED_ENTITY_ID", authorized_entity_id_string, 1);
    setenv(
        "AGENTD_AUTHORIZED_ENTITY_PUBKEY", authorized_entity_pubkey_string, 1);
    setenv("AGENTD_ID", agent_id_string, 1);
    setenv("AGENTD_PUBLIC_KEY", agent_pubkey_string, 1);
    setenv("AGENTD_PRIVATE_KEY", agent_privkey_string, 1);

    /* log to standard error. */
    logsock = dup(STDERR_FILENO);
    rlogsock = dup(STDERR_FILENO);

    /* create the socket pair for the datasock. */
    int datasock_srv;
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &datasock, &datasock_srv);

    /* create the socket pair for the acceptsock. */
    int acceptsock_srv;
    ipc_socketpair(AF_UNIX, SOCK_DGRAM, 0, &acceptsock, &acceptsock_srv);

    /* create the bootstrap config. */
    bootstrap_config_init(&bconf);

    /* set the default config. */
    memset(&conf, 0, sizeof(conf));
    conf.hdr.dispose = &config_dispose;

    /* spawn the random service process. */
    random_proc_status =
        randomservice_proc(
            &bconf, &conf, rlogsock, &rprotosock, &randompid, false);

    /* spawn the unauthorized protocol service process. */
    proto_proc_status =
        unauthorized_protocol_proc(
            &bconf, &conf, rprotosock, logsock, acceptsock_srv, datasock_srv,
            &protopid, false);

    /* if the spawn is successful, send the service the other half of a protocol
     * socket. */
    if (0 == proto_proc_status)
    {
        int protosock_srv;
        ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &protosock, &protosock_srv);
        ipc_sendsocket_block(acceptsock, protosock_srv);
        close(protosock_srv);
    }

    /* set up directory test helper. */
    string dbpath(wd);
    dbpath += "/build/test/isolation/databases/";
    directory_test_helper::SetUp(dir_key, dbpath.c_str());
}

void unauthorized_protocol_service_isolation_test::TearDown()
{
    directory_test_helper::TearDown();

    /* terminate the random service. */
    if (0 == random_proc_status)
    {
        int status = 0;
        kill(randompid, SIGTERM);
        waitpid(randompid, &status, 0);
    }

    /* terminate the unauthorized protocol service process. */
    if (0 == proto_proc_status)
    {
        int status = 0;
        close(protosock);
        kill(protopid, SIGTERM);
        waitpid(protopid, &status, 0);
    }

    /* set the old path. */
    setenv("PATH", oldpath, 1);

    /* clean up. */
    dispose((disposable_t*)&conf);
    dispose((disposable_t*)&bconf);
    close(logsock);
    close(rlogsock);
    close(datasock);
    close(acceptsock);
    free(path);
    if (suite_instance_initialized)
    {
        dispose((disposable_t*)&suite);
    }
    if (client_private_key_initialized)
    {
        dispose((disposable_t*)&client_private_key);
    }
    dispose((disposable_t*)&alloc_opts);
}
