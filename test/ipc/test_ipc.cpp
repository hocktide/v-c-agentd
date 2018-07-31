/**
 * \file test_ipc.cpp
 *
 * Test ipc methods.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <gtest/gtest.h>
#include <vpr/disposable.h>

using namespace std;

/**
 * \brief Calling ipc_make_block on a socket should make it blocking.
 */
TEST(ipc, ipc_make_block)
{
    int flags;
    int lhs, rhs;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* set the lhs socket to non-blocking using fcntl. */
    flags = fcntl(lhs, F_GETFL);
    ASSERT_LE(0, flags);
    flags |= O_NONBLOCK;
    ASSERT_LE(0, fcntl(lhs, F_SETFL, flags));

    /* precondition: lhs is non-blocking. */
    flags = fcntl(lhs, F_GETFL);
    ASSERT_LE(0, flags);
    ASSERT_EQ(O_NONBLOCK, flags & O_NONBLOCK);

    /* set lhs socket to blocking. */
    ASSERT_EQ(0, ipc_make_block(lhs));

    /* postcondition: lhs is blocking. */
    flags = fcntl(lhs, F_GETFL);
    ASSERT_LE(0, flags);
    ASSERT_EQ(0, flags & O_NONBLOCK);

    /* clean up. */
    close(lhs);
    close(rhs);
}
