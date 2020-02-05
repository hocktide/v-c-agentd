/**
 * \file test_auth_service_isolation_helpers.cpp
 *
 * Helpers for the authservice isolation tests.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#include "../../src/authservice/auth_service_private.h"
#include "test_auth_service_isolation.h"

using namespace std;

const uint8_t auth_service_isolation_test::agent_id[16] = {
    0x3d, 0x96, 0x3f, 0x54, 0x83, 0xe2, 0x4b, 0x0d,
    0x86, 0xa1, 0x81, 0xb6, 0xaa, 0xaa, 0x5c, 0x1b
};

const uint8_t auth_service_isolation_test::agent_privkey[] = {
    0x77, 0x07, 0x6d, 0x0a, 0x73, 0x18, 0xa5, 0x7d,
    0x3c, 0x16, 0xc1, 0x72, 0x51, 0xb2, 0x66, 0x45,
    0xdf, 0x4c, 0x2f, 0x87, 0xeb, 0xc0, 0x99, 0x2a,
    0xb1, 0x77, 0xfb, 0xa5, 0x1d, 0xb9, 0x2c, 0x2a
};

const uint8_t auth_service_isolation_test::agent_pubkey[] = {
    0x85, 0x20, 0xf0, 0x09, 0x89, 0x30, 0xa7, 0x54,
    0x74, 0x8b, 0x7d, 0xdc, 0xb4, 0x3e, 0xf7, 0x5a,
    0x0d, 0xbf, 0x3a, 0x0d, 0x26, 0x38, 0x1a, 0xf4,
    0xeb, 0xa4, 0xa9, 0x8e, 0xaa, 0x9b, 0x4e, 0x6a
};

void auth_service_isolation_test::SetUp()
{
    /* register vccrypt stuff. */
    vccrypt_suite_register_velo_v1();
    vccrypt_block_register_AES_256_2X_CBC();

    /* create malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* create suite. */
    suite_init_result =
        vccrypt_suite_options_init(
            &crypto_suite, &alloc_opts,
            VCCRYPT_SUITE_VELO_V1);


    /* create bootstrap config. */
    bootstrap_config_init(&bconf);

    /* set the default config. */
    memset(&conf, 0, sizeof(conf));
    conf.hdr.dispose = &config_dispose;

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

    /* spawn the authservice process. */
    auth_service_proc_status =
        auth_service_proc(
            &bconf, &conf, logsock, &authsock, &auth_pid,
            false);

    /* by default, we run in blocking mode. */
    nonblockauthsock_configured = false;
}

void auth_service_isolation_test::TearDown()
{
    if (suite_init_result == 0)
    {
        dispose((disposable_t*)&crypto_suite);
    }

    dispose((disposable_t*)&alloc_opts);

    /* terminate the authservice process. */
    if (0 == auth_service_proc_status)
    {
        int status = 0;
        kill(auth_pid, SIGTERM);
        waitpid(auth_pid, &status, 0);
    }

    /* set the old path. */
    setenv("PATH", oldpath, 1);

    /* clean up. */
    close(logsock);
    dispose((disposable_t*)&bconf);
    free(path);
}

void auth_service_isolation_test::nonblockmode(
    function<void()> onRead, function<void()> onWrite)
{
    /* set the read/write callbacks. */
    this->onRead = onRead;
    this->onWrite = onWrite;

    /* handle a non-blocking event loop. */
    if (!nonblockauthsock_configured)
    {
        ipc_make_noblock(authsock, &nonblockauthsock, this);
        nonblockauthsock_configured = true;
        ipc_event_loop_init(&loop);
    }
    else
    {
        ipc_event_loop_remove(&loop, &nonblockauthsock);
    }

    ipc_set_readcb_noblock(&nonblockauthsock, &nonblock_read, NULL);
    ipc_set_writecb_noblock(&nonblockauthsock, &nonblock_write, NULL);
    ipc_event_loop_add(&loop, &nonblockauthsock);
    ipc_event_loop_run(&loop);
}

void auth_service_isolation_test::nonblock_read(
    ipc_socket_context_t*, int, void* ctx)
{
    auth_service_isolation_test* that = (auth_service_isolation_test*)ctx;

    that->onRead();
}

void auth_service_isolation_test::nonblock_write(
    ipc_socket_context_t*, int, void* ctx)
{
    auth_service_isolation_test* that = (auth_service_isolation_test*)ctx;

    that->onWrite();
}
