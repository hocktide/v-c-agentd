/**
 * \file test_ipc.cpp
 *
 * Test ipc methods.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/inet.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdint.h>
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

/**
 * \brief It is possible to write a string value to a blocking socket.
 */
TEST(ipc, ipc_write_string_block)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    char buf[100];

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_string_block(lhs, TEST_STRING));

    /* read the type of the value from the rhs socket. */
    ASSERT_EQ(1, read(rhs, buf, 1));

    /* the type should be IPC_DATA_TYPE_STRING. */
    EXPECT_EQ(IPC_DATA_TYPE_STRING, buf[0]);

    /* read the size of the value from the rhs socket. */
    uint32_t nsize = 0U;
    ASSERT_EQ(sizeof(nsize), (uint32_t)read(rhs, &nsize, sizeof(nsize)));

    uint32_t size = ntohl(nsize);

    /* size should be the length of the string. */
    EXPECT_EQ(strlen(TEST_STRING), size);

    /* clear the buffer. */
    memset(buf, 0, sizeof(buf));

    /* read the string from the rhs socket. */
    ASSERT_EQ(size, (size_t)read(rhs, buf, size));

    /* the string value should match. */
    EXPECT_STREQ(TEST_STRING, buf);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief It is possible to write a uint64_t value to a blocking socket.
 */
TEST(ipc, ipc_write_uint64_block)
{
    int lhs, rhs;
    const uint64_t TEST_VAL = 98872;
    char buf[100];

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint64_block(lhs, TEST_VAL));

    /* read the type of the value from the rhs socket. */
    ASSERT_EQ(1, read(rhs, buf, 1));

    /* the type should be IPC_DATA_TYPE_UINT64. */
    EXPECT_EQ(IPC_DATA_TYPE_UINT64, buf[0]);

    /* read the size of the value from the rhs socket. */
    uint32_t nsize = 0U;
    ASSERT_EQ(sizeof(nsize), (uint32_t)read(rhs, &nsize, sizeof(nsize)));

    size_t size = ntohl(nsize);

    /* size should be the size of uint64_t. */
    EXPECT_EQ(sizeof(uint64_t), size);

    /* read the uint64_t value from the stream. */
    uint64_t nval = 0;
    ASSERT_EQ(sizeof(nval), (size_t)read(rhs, &nval, sizeof(nval)));

    /* swap the value to local endianness. */
    uint64_t val = ntohll(nval);

    /* the value should equal our original value. */
    EXPECT_EQ(TEST_VAL, val);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief It is possible to write an int64_t value to a blocking socket.
 */
TEST(ipc, ipc_write_int64_block)
{
    int lhs, rhs;
    const int64_t TEST_VAL = -98872;
    char buf[100];

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_int64_block(lhs, TEST_VAL));

    /* read the type of the value from the rhs socket. */
    ASSERT_EQ(1, read(rhs, buf, 1));

    /* the type should be IPC_DATA_TYPE_INT64. */
    EXPECT_EQ(IPC_DATA_TYPE_INT64, buf[0]);

    /* read the size of the value from the rhs socket. */
    uint32_t nsize = 0U;
    ASSERT_EQ(sizeof(nsize), (uint32_t)read(rhs, &nsize, sizeof(nsize)));

    size_t size = ntohl(nsize);

    /* size should be the size of int64_t. */
    EXPECT_EQ(sizeof(int64_t), size);

    /* read the int64_t value from the stream. */
    int64_t nval = 0;
    ASSERT_EQ(sizeof(nval), (size_t)read(rhs, &nval, sizeof(nval)));

    /* swap the value to local endianness. */
    int64_t val = ntohll(nval);

    /* the value should equal our original value. */
    EXPECT_EQ(TEST_VAL, val);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief It is possible to write a uint8_t value to a blocking socket.
 */
TEST(ipc, ipc_write_uint8_block)
{
    int lhs, rhs;
    const uint8_t TEST_VAL = 76;
    char buf[100];

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint8_block(lhs, TEST_VAL));

    /* read the type of the value from the rhs socket. */
    ASSERT_EQ(1, read(rhs, buf, 1));

    /* the type should be IPC_DATA_TYPE_UINT8. */
    EXPECT_EQ(IPC_DATA_TYPE_UINT8, buf[0]);

    /* read the size of the value from the rhs socket. */
    uint32_t nsize = 0U;
    ASSERT_EQ(sizeof(nsize), (uint32_t)read(rhs, &nsize, sizeof(nsize)));

    size_t size = ntohl(nsize);

    /* size should be the size of uint8_t. */
    EXPECT_EQ(sizeof(uint8_t), size);

    /* read the uint8_t value from the stream. */
    uint8_t val = 0;
    ASSERT_EQ(sizeof(val), (size_t)read(rhs, &val, sizeof(val)));

    /* the value should equal our original value. */
    EXPECT_EQ(TEST_VAL, val);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief It is possible to write an int8_t value to a blocking socket.
 */
TEST(ipc, ipc_write_int8_block)
{
    int lhs, rhs;
    const int8_t TEST_VAL = -76;
    char buf[100];

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_int8_block(lhs, TEST_VAL));

    /* read the type of the value from the rhs socket. */
    ASSERT_EQ(1, read(rhs, buf, 1));

    /* the type should be IPC_DATA_TYPE_INT8. */
    EXPECT_EQ(IPC_DATA_TYPE_INT8, buf[0]);

    /* read the size of the value from the rhs socket. */
    uint32_t nsize = 0U;
    ASSERT_EQ(sizeof(nsize), (uint32_t)read(rhs, &nsize, sizeof(nsize)));

    size_t size = ntohl(nsize);

    /* size should be the size of int8_t. */
    EXPECT_EQ(sizeof(int8_t), size);

    /* read the int8_t value from the stream. */
    int8_t val = 0;
    ASSERT_EQ(sizeof(val), (size_t)read(rhs, &val, sizeof(val)));

    /* the value should equal our original value. */
    EXPECT_EQ(TEST_VAL, val);

    /* clean up. */
    close(lhs);
    close(rhs);
}
