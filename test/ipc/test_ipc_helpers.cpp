/**
 * \file test_ipc_helpers.cpp
 *
 * Helpers used by the ipc unit tests.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#include <vpr/allocator/malloc_allocator.h>

#include "test_ipc.h"

using namespace std;

/**
 * \brief Set up a unit test.
 */
void ipc_test::SetUp()
{
    /* register the Velo V1 crypto suite. */
    vccrypt_suite_register_velo_v1();

    /* initialize the malloc allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* initialize the crypto suite. */
    int retval =
        vccrypt_suite_options_init(&suite, &alloc_opts, VCCRYPT_SUITE_VELO_V1);
    if (0 == retval)
    {
        suite_configured = true;
    }

    /* by default, we run in blocking mode. */
    nonblockdatasock_configured = false;
}

/**
 * \brief Tear down a unit test.
 */
void ipc_test::TearDown()
{
    if (nonblockdatasock_configured)
    {
        dispose((disposable_t*)&nonblockdatasock);
        nonblockdatasock_configured = false;
    }

    if (suite_configured)
    {
        dispose((disposable_t*)&suite);
        suite_configured = false;
    }

    dispose((disposable_t*)&alloc_opts);
}

/**
 * Run socket code in non-blocking mode.
 */
void ipc_test::nonblockmode(
    int datasock,
    function<void()> onRead, function<void()> onWrite)
{
    /* set the read/write callbacks. */
    this->onRead = onRead;
    this->onWrite = onWrite;

    /* cleanup from previous run. */
    if (nonblockdatasock_configured)
    {
        ipc_event_loop_remove(&loop, &nonblockdatasock);
        dispose((disposable_t*)&nonblockdatasock);
        nonblockdatasock_configured = false;
    }

    /* handle a non-blocking event loop. */
    if (!nonblockdatasock_configured)
    {
        ipc_make_noblock(datasock, &nonblockdatasock, this);
        nonblockdatasock_configured = true;
        ipc_event_loop_init(&loop);
    }

    ipc_set_readcb_noblock(&nonblockdatasock, &nonblock_read, NULL);
    ipc_set_writecb_noblock(&nonblockdatasock, &nonblock_write, NULL);
    ipc_event_loop_add(&loop, &nonblockdatasock);
    ipc_event_loop_run(&loop);
}

void ipc_test::nonblock_read(
    ipc_socket_context_t*, int, void* ctx)
{
    ipc_test* that = (ipc_test*)ctx;

    that->onRead();
}

void ipc_test::nonblock_write(
    ipc_socket_context_t*, int, void* ctx)
{
    ipc_test* that = (ipc_test*)ctx;

    that->onWrite();
}
