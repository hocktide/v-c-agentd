/**
 * \file test_ipc_helpers.cpp
 *
 * Helpers used by the ipc unit tests.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#include "test_ipc.h"

using namespace std;

/**
 * \brief Set up a unit test.
 */
void ipc_test::SetUp()
{
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

    ipc_set_readcb_noblock(&nonblockdatasock, &nonblock_read);
    ipc_set_writecb_noblock(&nonblockdatasock, &nonblock_write);
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
