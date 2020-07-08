/**
 * \file test_ipc.cpp
 *
 * Test ipc methods.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/socket.h>
#include <time.h>
#include <vpr/disposable.h>

#include "test_ipc.h"

using namespace std;

/**
 * \brief Calling ipc_make_block on a socket should make it blocking.
 */
TEST_F(ipc_test, ipc_make_block)
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
TEST_F(ipc_test, ipc_write_string_block)
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
 * \brief It is possible to write a data value to a blocking socket.
 */
TEST_F(ipc_test, ipc_write_data_block)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    char buf[100];

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a data block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_data_block(lhs, TEST_STRING, strlen(TEST_STRING)));

    /* read the type of the value from the rhs socket. */
    ASSERT_EQ(1, read(rhs, buf, 1));

    /* the type should be IPC_DATA_TYPE_DATA_PACKET. */
    EXPECT_EQ(IPC_DATA_TYPE_DATA_PACKET, buf[0]);

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
TEST_F(ipc_test, ipc_write_uint64_block)
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
TEST_F(ipc_test, ipc_write_int64_block)
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
TEST_F(ipc_test, ipc_write_uint8_block)
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
TEST_F(ipc_test, ipc_write_int8_block)
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

/**
 * \brief It is possible to read a string value from a blocking socket.
 */
TEST_F(ipc_test, ipc_read_string_block_success)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    char* str = nullptr;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_string_block(lhs, TEST_STRING));

    /* read a string block from the rhs socket. */
    ASSERT_EQ(0, ipc_read_string_block(rhs, &str));

    /* the string is valid. */
    ASSERT_NE(nullptr, str);

    /* the string is a copy of the test string. */
    EXPECT_STREQ(TEST_STRING, str);

    /* clean up. */
    free(str);
    close(lhs);
    close(rhs);
}

/**
 * \brief If another value is seen instead of a string, fail.
 */
TEST_F(ipc_test, ipc_read_string_block_bad_type)
{
    int lhs, rhs;
    uint64_t badval = 1;
    char* str = nullptr;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint64_block(lhs, badval));

    /* read a string block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_string_block(rhs, &str));

    /* the string is NULL. */
    ASSERT_EQ(nullptr, str);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief If the connection is reset before reading type, return an error.
 */
TEST_F(ipc_test, ipc_read_string_block_reset_connection_1)
{
    int lhs, rhs;
    char* str = nullptr;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* reset the peer connection. */
    close(lhs);

    /* read a string block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_string_block(rhs, &str));

    /* the string is NULL. */
    ASSERT_EQ(nullptr, str);

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the size is not read, fail.
 */
TEST_F(ipc_test, ipc_read_string_block_bad_size)
{
    int lhs, rhs;
    char* str = nullptr;
    uint8_t type = IPC_DATA_TYPE_STRING;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the string type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* close the lhs socket. */
    close(lhs);

    /* read a string block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_string_block(rhs, &str));

    /* the string is NULL. */
    ASSERT_EQ(nullptr, str);

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the string is not read, fail.
 */
TEST_F(ipc_test, ipc_read_string_block_bad_data)
{
    int lhs, rhs;
    char* str = nullptr;
    uint8_t type = IPC_DATA_TYPE_STRING;
    uint32_t size = htonl(10);

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the string type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* write the string size to the lhs socket. */
    ASSERT_EQ(sizeof(size), (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* read a string block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_string_block(rhs, &str));

    /* the string is NULL. */
    ASSERT_EQ(nullptr, str);

    /* clean up. */
    close(rhs);
}

/**
 * \brief It is possible to read a data packet from a blocking socket.
 */
TEST_F(ipc_test, ipc_read_data_block_success)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    void* str = nullptr;
    uint32_t str_size = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_data_block(lhs, TEST_STRING, strlen(TEST_STRING)));

    /* read a data packet from the rhs socket. */
    ASSERT_EQ(0, ipc_read_data_block(rhs, &str, &str_size));

    /* the data is valid. */
    ASSERT_NE(nullptr, str);

    /* the string size is the length of our string. */
    ASSERT_EQ(strlen(TEST_STRING), str_size);

    /* the data is a copy of the test string. */
    EXPECT_EQ(0, memcmp(TEST_STRING, str, str_size));

    /* clean up. */
    free(str);
    close(lhs);
    close(rhs);
}

/**
 * \brief It is possible to read a uint64_t value from a blocking socket.
 */
TEST_F(ipc_test, ipc_read_uint64_block_success)
{
    int lhs, rhs;
    uint64_t val = 910028;
    uint64_t read_val = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint64_t block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint64_block(lhs, val));

    /* read a uint64_t block from the rhs socket. */
    ASSERT_EQ(0, ipc_read_uint64_block(rhs, &read_val));

    /* the value is a copy of the test value. */
    EXPECT_EQ(val, read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief If another value is seen instead of a uint64_t, fail.
 */
TEST_F(ipc_test, ipc_read_uint64_block_bad_type)
{
    int lhs, rhs;
    uint8_t badval = 1U;
    uint64_t read_val = 0U;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint8_t block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint8_block(lhs, badval));

    /* reading a uint64 block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_uint64_block(rhs, &read_val));

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief If the peer socket is reset before the type is written, return an
 * error.
 */
TEST_F(ipc_test, ipc_read_uint64_reset_connection_1)
{
    int lhs, rhs;
    uint64_t read_val = 0U;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* close the peer socket. */
    close(lhs);

    /* reading a uint64 block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_uint64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the peer socket is reset before the size is written, return an
 * error.
 */
TEST_F(ipc_test, ipc_read_uint64_reset_connection_2)
{
    int lhs, rhs;
    uint64_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_UINT64;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the uint64 type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a uint64_t block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_uint64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the size is invalid, return an error.
 */
TEST_F(ipc_test, ipc_read_uint64_block_bad_size)
{
    int lhs, rhs;
    uint64_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_UINT64;
    uint32_t size = htonl(99);

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the uint64_t type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* write the uint64_t size to the lhs socket. */
    ASSERT_EQ(sizeof(size), (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a uint64_t block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_uint64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the value is not read, fail.
 */
TEST_F(ipc_test, ipc_read_uint64_block_bad_data)
{
    int lhs, rhs;
    uint64_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_UINT64;
    uint32_t size = htonl(sizeof(read_val));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the uint64_t type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* write the uint64_t size to the lhs socket. */
    ASSERT_EQ(sizeof(size), (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a uint64_t block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_uint64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief It is possible to read a int64_t value from a blocking socket.
 */
TEST_F(ipc_test, ipc_read_int64_block_success)
{
    int lhs, rhs;
    int64_t val = -910028;
    int64_t read_val = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a int64_t block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_int64_block(lhs, val));

    /* read a int64_t block from the rhs socket. */
    ASSERT_EQ(0, ipc_read_int64_block(rhs, &read_val));

    /* the value is a copy of the test value. */
    EXPECT_EQ(val, read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief If the peer connection is reset before the type is written, then
 * return an error.
 */
TEST_F(ipc_test, ipc_read_int64_block_reset_connection_1)
{
    int lhs, rhs;
    int64_t read_val = 0U;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* close the connection before writing the type. */
    close(lhs);

    /* reading a int64 block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_int64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If another value is seen instead of a int64_t, fail.
 */
TEST_F(ipc_test, ipc_read_int64_block_bad_type)
{
    int lhs, rhs;
    uint8_t badval = 1U;
    int64_t read_val = 0U;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint8_t block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint8_block(lhs, badval));

    /* reading a int64 block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_int64_block(rhs, &read_val));

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief If the connection is closed before the size is written, return an
 * error.
 */
TEST_F(ipc_test, ipc_read_int64_block_reset_connection_2)
{
    int lhs, rhs;
    int64_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_INT64;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the int64 type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a int64_t block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_int64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If a bad size is given, return an error.
 */
TEST_F(ipc_test, ipc_read_int64_block_bad_size)
{
    int lhs, rhs;
    int64_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_INT64;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the int64 type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* write a bad size to the lhs socket. */
    uint32_t size = htonl(99);
    ASSERT_EQ(sizeof(size), (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a int64_t block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_int64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the connection is closed before the data is written, return an
 * error.
 */
TEST_F(ipc_test, ipc_read_int64_block_reset_connection_3)
{
    int lhs, rhs;
    int64_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_INT64;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the int64 type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* write a valid size. */
    uint32_t size = htonl(sizeof(int64_t));
    ASSERT_EQ(sizeof(size), (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a int64_t block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_int64_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief It is possible to read a uint8_t value from a blocking socket.
 */
TEST_F(ipc_test, ipc_read_uint8_block_success)
{
    int lhs, rhs;
    uint8_t val = 28;
    uint8_t read_val = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint8_t block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint8_block(lhs, val));

    /* read a uint8_t block from the rhs socket. */
    ASSERT_EQ(0, ipc_read_uint8_block(rhs, &read_val));

    /* the value is a copy of the test value. */
    EXPECT_EQ(val, read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief If another value is seen instead of a uint8_t, fail.
 */
TEST_F(ipc_test, ipc_read_uint8_block_bad_type)
{
    int lhs, rhs;
    uint64_t badval = 1U;
    uint8_t read_val = 0U;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint64_t block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint64_block(lhs, badval));

    /* reading a uint8 block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_uint8_block(rhs, &read_val));

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief If the socket connection is reset prior to reading the type, return an
 * error.
 */
TEST_F(ipc_test, ipc_read_uint8_reset_connection_1)
{
    int lhs, rhs;
    uint8_t read_val = 0U;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* close the peer socket. */
    close(lhs);

    /* reading a uint8 block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_uint8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the size is not read, fail.
 */
TEST_F(ipc_test, ipc_read_uint8_block_bad_size)
{
    int lhs, rhs;
    uint8_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_UINT8;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the uint8 type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a uint8_t block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_uint8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the socket connection is reset prior to reading the value, return
 * an error.
 */
TEST_F(ipc_test, ipc_read_uint8_reset_connection_2)
{
    int lhs, rhs;
    uint8_t read_val = 0U;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the type. */
    uint8_t type = IPC_DATA_TYPE_UINT8;
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* close the peer socket. */
    close(lhs);

    /* reading a uint8 block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_uint8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the size is invalid, return an error.
 */
TEST_F(ipc_test, ipc_read_uint8_bad_size)
{
    int lhs, rhs;
    uint8_t read_val = 0U;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the type. */
    uint8_t type = IPC_DATA_TYPE_UINT8;
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* write the size. */
    uint32_t size = htonl(12);
    ASSERT_EQ(sizeof(size), (size_t)write(lhs, &size, sizeof(size)));

    /* close the peer socket. */
    close(lhs);

    /* reading a uint8 block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_uint8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the value is not read, fail.
 */
TEST_F(ipc_test, ipc_read_uint8_block_bad_data)
{
    int lhs, rhs;
    uint8_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_UINT8;
    uint32_t size = htonl(sizeof(read_val));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the uint8_t type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* write the uint8_t size to the lhs socket. */
    ASSERT_EQ(sizeof(size), (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a uint8_t block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_uint8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief It is possible to read a int8_t value from a blocking socket.
 */
TEST_F(ipc_test, ipc_read_int8_block_success)
{
    int lhs, rhs;
    int8_t val = 28;
    int8_t read_val = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a int8_t block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_int8_block(lhs, val));

    /* read a int8_t block from the rhs socket. */
    ASSERT_EQ(0, ipc_read_int8_block(rhs, &read_val));

    /* the value is a copy of the test value. */
    EXPECT_EQ(val, read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief If the peer connection is reset, the int8 read fails.
 */
TEST_F(ipc_test, ipc_read_int8_block_reset_connection_1)
{
    int lhs, rhs;
    int8_t read_val = 0U;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* close the peer socket. */
    close(lhs);

    /* reading a int8 block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_int8_block(rhs, &read_val));

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief If another value is seen instead of a int8_t, fail.
 */
TEST_F(ipc_test, ipc_read_int8_block_bad_type)
{
    int lhs, rhs;
    uint64_t badval = 1U;
    int8_t read_val = 0U;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint64_t block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint64_block(lhs, badval));

    /* reading a int8 block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_int8_block(rhs, &read_val));

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief If the peer connection is reset prior to writing size, an error code
 * is returned.
 */
TEST_F(ipc_test, ipc_read_int8_reset_connection_2)
{
    int lhs, rhs;
    int8_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_INT8;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the int8 type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a int8_t block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_int8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the size is invalid, return an error.
 */
TEST_F(ipc_test, ipc_read_int8_bad_size)
{
    int lhs, rhs;
    int8_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_INT8;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the int8 type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* write a bad size. */
    uint32_t size = htonl(12);
    ASSERT_EQ(sizeof(size), (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a int8_t block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_int8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the value is not read, fail.
 */
TEST_F(ipc_test, ipc_read_int8_block_bad_data)
{
    int lhs, rhs;
    int8_t read_val = 0U;
    uint8_t type = IPC_DATA_TYPE_INT8;
    uint32_t size = htonl(sizeof(read_val));

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the int8_t type to the lhs socket. */
    ASSERT_EQ(sizeof(type), (size_t)write(lhs, &type, sizeof(type)));

    /* write the int8_t size to the lhs socket. */
    ASSERT_EQ(sizeof(size), (size_t)write(lhs, &size, sizeof(size)));

    /* close the lhs socket. */
    close(lhs);

    /* reading a int8_t block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_int8_block(rhs, &read_val));

    /* clean up. */
    close(rhs);
}

/**
 * \brief If another value is seen instead of a data packet, fail.
 */
TEST_F(ipc_test, ipc_read_data_block_bad_type)
{
    int lhs, rhs;
    uint64_t badval = 1;
    void* str = nullptr;
    uint32_t str_size = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a string block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint64_block(lhs, badval));

    /* read a string block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_data_block(rhs, &str, &str_size));

    /* the string is NULL. */
    ASSERT_EQ(nullptr, str);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief If the socket is closed before a data block is written, it fails.
 */
TEST_F(ipc_test, ipc_read_data_block_connection_reset_1)
{
    int lhs, rhs;
    void* str = nullptr;
    uint32_t str_size = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* close the lhs socket. */
    close(lhs);

    /* read a string block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_data_block(rhs, &str, &str_size));

    /* the string is NULL. */
    ASSERT_EQ(nullptr, str);

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the socket is closed in the middle of a write, reading fails.
 */
TEST_F(ipc_test, ipc_read_data_block_connection_reset_2)
{
    int lhs, rhs;
    void* str = nullptr;
    uint32_t str_size = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the packet type to the socket. */
    const uint8_t type = IPC_DATA_TYPE_DATA_PACKET;
    ASSERT_EQ(1, write(lhs, &type, sizeof(type)));

    /* close the lhs socket. */
    close(lhs);

    /* read a string block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_data_block(rhs, &str, &str_size));

    /* the string is NULL. */
    ASSERT_EQ(nullptr, str);

    /* clean up. */
    close(rhs);
}

/**
 * \brief If the socket is closed in the middle of a write, reading fails.
 */
TEST_F(ipc_test, ipc_read_data_block_connection_reset_3)
{
    int lhs, rhs;
    void* str = nullptr;
    uint32_t str_size = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write the packet type to the socket. */
    const uint8_t type = IPC_DATA_TYPE_DATA_PACKET;
    ASSERT_EQ(1, write(lhs, &type, sizeof(type)));

    /* write the packet length to the socket. */
    uint32_t packet_len = htonl(10);
    ASSERT_EQ(4, write(lhs, &packet_len, sizeof(packet_len)));

    /* close the lhs socket. */
    close(lhs);

    /* read a string block from the rhs socket fails. */
    ASSERT_NE(0, ipc_read_data_block(rhs, &str, &str_size));

    /* the string is NULL. */
    ASSERT_EQ(nullptr, str);

    /* clean up. */
    close(rhs);
}

/**
 * \brief It is possible to read a uint8_t value from a non-blocking socket.
 */
TEST_F(ipc_test, ipc_read_uint8_noblock_success)
{
    int lhs, rhs;
    uint8_t val = 28;
    uint8_t read_val = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint8_t block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint8_block(lhs, val));

    int read_resp = AGENTD_ERROR_IPC_WOULD_BLOCK;

    nonblockmode(
        rhs,
        /* onRead */
        [&]() {
            if (AGENTD_ERROR_IPC_WOULD_BLOCK == read_resp)
            {
                read_resp =
                    ipc_read_uint8_noblock(&nonblockdatasock, &read_val);

                if (read_resp != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite */
        [&]() {
        });

    /* read should have succeeded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, read_resp);
    /* we read a valid uint8_t. */
    EXPECT_EQ(val, read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief It is possible to read an int8_t value from a non-blocking socket.
 */
TEST_F(ipc_test, ipc_read_int8_noblock_success)
{
    int lhs, rhs;
    int8_t val = 28;
    int8_t read_val = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write an int8_t block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_int8_block(lhs, val));

    int read_resp = AGENTD_ERROR_IPC_WOULD_BLOCK;

    nonblockmode(
        rhs,
        /* onRead */
        [&]() {
            if (AGENTD_ERROR_IPC_WOULD_BLOCK == read_resp)
            {
                read_resp =
                    ipc_read_int8_noblock(&nonblockdatasock, &read_val);

                if (read_resp != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite */
        [&]() {
        });

    /* read should have succeeded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, read_resp);
    /* we read a valid uint8_t. */
    EXPECT_EQ(val, read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief It is possible to read a uint64_t value from a non-blocking socket.
 */
TEST_F(ipc_test, ipc_read_uint64_noblock_success)
{
    int lhs, rhs;
    uint64_t val = 28;
    uint64_t read_val = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write a uint64_t block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_uint64_block(lhs, val));

    int read_resp = AGENTD_ERROR_IPC_WOULD_BLOCK;

    nonblockmode(
        rhs,
        /* onRead */
        [&]() {
            if (AGENTD_ERROR_IPC_WOULD_BLOCK == read_resp)
            {
                read_resp =
                    ipc_read_uint64_noblock(&nonblockdatasock, &read_val);

                if (read_resp != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite */
        [&]() {
        });

    /* read should have succeeded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, read_resp);
    /* we read a valid uint8_t. */
    EXPECT_EQ(val, read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief It is possible to read a int64_t value from a non-blocking socket.
 */
TEST_F(ipc_test, ipc_read_int64_noblock_success)
{
    int lhs, rhs;
    int64_t val = 28;
    int64_t read_val = 0;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* write an int64_t block to the lhs socket. */
    ASSERT_EQ(0, ipc_write_int64_block(lhs, val));

    int read_resp = AGENTD_ERROR_IPC_WOULD_BLOCK;

    nonblockmode(
        rhs,
        /* onRead */
        [&]() {
            if (AGENTD_ERROR_IPC_WOULD_BLOCK == read_resp)
            {
                read_resp =
                    ipc_read_int64_noblock(&nonblockdatasock, &read_val);

                if (read_resp != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite */
        [&]() {
        });

    /* read should have succeeded. */
    ASSERT_EQ(AGENTD_STATUS_SUCCESS, read_resp);
    /* we read a valid uint8_t. */
    EXPECT_EQ(val, read_val);

    /* clean up. */
    close(lhs);
    close(rhs);
}

/**
 * \brief It is possible to read an authed packet from a blocking socket.
 */
TEST_F(ipc_test, ipc_read_authed_block_success)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    void* str = nullptr;
    uint32_t str_size = 0;
    constexpr size_t ENC_PAYLOAD_SIZE =
        sizeof(uint8_t) +  //type
        sizeof(uint32_t) +  //size
        32 +  //hmac
        15;  //string length
    char TEST_PAYLOAD[ENC_PAYLOAD_SIZE] = { 0 };
    uint64_t iv = 12345;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* create key for stream cipher. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t key;
    ASSERT_EQ(
        0,
        vccrypt_buffer_init(
            &key, &alloc_opts, suite.stream_cipher_opts.key_size));

    /* set a null key. */
    memset(key.data, 0, key.size);

    /* create a stream cipher instance. */
    vccrypt_stream_context_t stream;
    ASSERT_EQ(0, vccrypt_suite_stream_init(&suite, &stream, &key));

    /* create a MAC instance. */
    vccrypt_mac_context_t mac;
    ASSERT_EQ(0, vccrypt_suite_mac_short_init(&suite, &mac, &key));

    /* create a MAC digest buffer. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t digest;
    ASSERT_EQ(0,
        vccrypt_buffer_init(
            &digest, &alloc_opts, suite.mac_short_opts.mac_size));

    /* continue encryption from the current iv, offset 0. */
    ASSERT_EQ(
        0,
        vccrypt_stream_continue_encryption(&stream, &iv, sizeof(iv), 0));

    /* write the packet type to the buffer. */
    uint8_t type = IPC_DATA_TYPE_AUTHED_PACKET;
    size_t offset = 0;
    ASSERT_EQ(0,
        vccrypt_stream_encrypt(
            &stream, &type, sizeof(type), TEST_PAYLOAD, &offset));
    /* digest the packet type. */
    ASSERT_EQ(0,
        vccrypt_mac_digest(
            &mac, (const uint8_t*)TEST_PAYLOAD + offset - sizeof(type),
            sizeof(type)));

    /* write the payload size to the buffer. */
    uint32_t payload_size = htonl(15);
    ASSERT_EQ(0,
        vccrypt_stream_encrypt(
            &stream, &payload_size, sizeof(payload_size), TEST_PAYLOAD,
            &offset));
    /* digest the payload size. */
    ASSERT_EQ(0,
        vccrypt_mac_digest(
            &mac, (const uint8_t*)TEST_PAYLOAD + offset - sizeof(payload_size),
            sizeof(payload_size)));

    /* write the payload to the buffer, skipping the hmac. */
    ASSERT_EQ(0,
        vccrypt_stream_encrypt(
            &stream, TEST_STRING, 15, TEST_PAYLOAD + 32, &offset));
    /* digest the payload. */
    ASSERT_EQ(0,
        vccrypt_mac_digest(
            &mac, (const uint8_t*)TEST_PAYLOAD + 32 + offset - 15, 15));

    /* finalize the mac to the test payload. */
    ASSERT_EQ(0, vccrypt_mac_finalize(&mac, &digest));
    memcpy(
        TEST_PAYLOAD + sizeof(uint8_t) + sizeof(uint32_t), digest.data,
        digest.size);

    /* write the payload to the lhs socket. */
    ASSERT_EQ((ssize_t)sizeof(TEST_PAYLOAD),
        write(lhs, TEST_PAYLOAD, sizeof(TEST_PAYLOAD)));

    /* read an authed packet from the rhs socket. */
    ASSERT_EQ(0,
        ipc_read_authed_data_block(rhs, iv, &str, &str_size, &suite, &key));

    /* the data is valid. */
    ASSERT_NE(nullptr, str);

    /* the string size is the length of our string. */
    ASSERT_EQ(strlen(TEST_STRING), str_size);

    /* the data is a copy of the test string. */
    EXPECT_EQ(0, memcmp(TEST_STRING, str, str_size));

    /* clean up. */
    free(str);
    close(lhs);
    close(rhs);
    dispose((disposable_t*)&key);
    dispose((disposable_t*)&stream);
    dispose((disposable_t*)&mac);
    dispose((disposable_t*)&digest);
}

/**
 * \brief It is possible to read an authed packet from a blocking socket that
 * was written by ipc_write_authed_block.
 */
TEST_F(ipc_test, ipc_write_authed_block_success)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    void* str = nullptr;
    uint32_t str_size = 0;
    uint64_t iv = 12345;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* create key for stream cipher. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t key;
    ASSERT_EQ(
        0,
        vccrypt_buffer_init(
            &key, &alloc_opts, suite.stream_cipher_opts.key_size));

    /* set a null key. */
    memset(key.data, 0, key.size);

    /* writing to the socket should succeed. */
    ASSERT_EQ(
        0,
        ipc_write_authed_data_block(
            lhs, iv, TEST_STRING, strlen(TEST_STRING), &suite, &key));

    /* read an authed packet from the rhs socket. */
    ASSERT_EQ(0,
        ipc_read_authed_data_block(rhs, iv, &str, &str_size, &suite, &key));

    /* the data is valid. */
    ASSERT_NE(nullptr, str);

    /* the string size is the length of our string. */
    ASSERT_EQ(strlen(TEST_STRING), str_size);

    /* the data is a copy of the test string. */
    EXPECT_EQ(0, memcmp(TEST_STRING, str, str_size));

    /* clean up. */
    free(str);
    close(lhs);
    close(rhs);
    dispose((disposable_t*)&key);
}

/**
 * \brief It is possible to read an authed packet from a non-blocking socket
 * that was written by ipc_write_authed_block.
 */
TEST_F(ipc_test, ipc_read_authed_noblock_success)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    void* str = nullptr;
    uint32_t str_size = 0;
    uint64_t iv = 12345;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* create key for stream cipher. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t key;
    ASSERT_EQ(
        0,
        vccrypt_buffer_init(
            &key, &alloc_opts, suite.stream_cipher_opts.key_size));

    /* set a null key. */
    memset(key.data, 0, key.size);

    /* writing to the socket should succeed. */
    ASSERT_EQ(
        0,
        ipc_write_authed_data_block(
            lhs, iv, TEST_STRING, strlen(TEST_STRING), &suite, &key));

    int read_resp = AGENTD_ERROR_IPC_WOULD_BLOCK;

    /* read an authed packet from the rhs socket. */
    nonblockmode(
        rhs,
        /* onRead */
        [&]() {
            if (AGENTD_ERROR_IPC_WOULD_BLOCK == read_resp)
            {
                read_resp =
                    ipc_read_authed_data_noblock(
                        &nonblockdatasock, iv, &str, &str_size, &suite, &key);

                if (read_resp != AGENTD_ERROR_IPC_WOULD_BLOCK)
                {
                    ipc_exit_loop(&loop);
                }
            }
        },
        /* onWrite */
        [&]() {
        });

    /* read should have succeeded. */
    ASSERT_EQ(0, read_resp);
    /* the data is valid. */
    ASSERT_NE(nullptr, str);

    /* the string size is the length of our string. */
    ASSERT_EQ(strlen(TEST_STRING), str_size);

    /* the data is a copy of the test string. */
    EXPECT_EQ(0, memcmp(TEST_STRING, str, str_size));

    /* clean up. */
    free(str);
    close(lhs);
    close(rhs);
    dispose((disposable_t*)&key);
}

/**
 * \brief It is possible to write a packet via ipc_write_authed_noblock and read
 * it using ipc_read_authed_block.
 */
TEST_F(ipc_test, ipc_write_authed_noblock_success)
{
    int lhs, rhs;
    const char TEST_STRING[] = "This is a test.";
    void* str = nullptr;
    uint32_t str_size = 0;
    uint64_t iv = 12345;

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* create key for stream cipher. */
    /* TODO - there should be a suite method for this. */
    vccrypt_buffer_t key;
    ASSERT_EQ(
        0,
        vccrypt_buffer_init(
            &key, &alloc_opts, suite.stream_cipher_opts.key_size));

    /* set a null key. */
    memset(key.data, 0, key.size);

    int write_resp = AGENTD_ERROR_IPC_WOULD_BLOCK;

    /* writing to the socket should succeed. */
    nonblockmode(
        lhs,
        /* onRead */
        [&]() {
        },
        /* onWrite */
        [&]() {
            if (AGENTD_ERROR_IPC_WOULD_BLOCK == write_resp)
            {
                write_resp =
                    ipc_write_authed_data_noblock(
                        &nonblockdatasock, iv, TEST_STRING, strlen(TEST_STRING),
                        &suite, &key);
            }
            else
            {
                if (ipc_socket_writebuffer_size(&nonblockdatasock) > 0)
                {
                    int bytes_written =
                        ipc_socket_write_from_buffer(&nonblockdatasock);

                    if (bytes_written == 0 || (bytes_written < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)))
                    {
                        ipc_exit_loop(&loop);
                    }
                }
                else
                {
                    ipc_exit_loop(&loop);
                }
            }
        });
    /* the write should have succeeded. */
    ASSERT_EQ(0, write_resp);

    /* read an authed packet from the rhs socket. */
    ASSERT_EQ(
        0,
        ipc_read_authed_data_block(rhs, iv, &str, &str_size, &suite, &key));
    /* the data is valid. */
    ASSERT_NE(nullptr, str);

    /* the string size is the length of our string. */
    ASSERT_EQ(strlen(TEST_STRING), str_size);

    /* the data is a copy of the test string. */
    EXPECT_EQ(0, memcmp(TEST_STRING, str, str_size));

    /* clean up. */
    free(str);
    close(lhs);
    close(rhs);
    dispose((disposable_t*)&key);
}

static void test_timer_cb(ipc_timer_context_t*, void* user_context)
{
    function<void()>* func = (function<void()>*)user_context;

    (*func)();
}

/**
 * \brief It is possible to create a timer and have it fire.
 */
TEST_F(ipc_test, ipc_timer)
{
    int lhs, rhs;
    bool callback_called = false;
    timespec start_time;
    timespec callback_time;
    timespec expected_time;
    ipc_timer_context_t timer;

    function<void()> callback = [&]() {
        ASSERT_EQ(0, clock_gettime(CLOCK_REALTIME, &callback_time));
        callback_called = true;
    };

    /* create a socket pair for testing. */
    ASSERT_EQ(0, ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs));

    /* set up the loop, using one of the sockets as a hack. */
    timermode_setup(lhs);

    /* initialize the timer event. */
    ASSERT_EQ(0, ipc_timer_init(&timer, 250, &test_timer_cb, &callback));

    /* add the timer to the loop. */
    ASSERT_EQ(0, ipc_event_loop_add_timer(&loop, &timer));

    /* get the current time. */
    ASSERT_EQ(0, clock_gettime(CLOCK_REALTIME, &start_time));

    /* run the loop. */
    timermode();

    /* verify that the callback was called. */
    ASSERT_TRUE(callback_called);

    /* we expect the callback to happen at least 250 milliseconds after
     * start_time. */
    memcpy(&expected_time, &start_time, sizeof(timespec));
    expected_time.tv_nsec += 250 * 1000 * 1000;
    expected_time.tv_sec += expected_time.tv_nsec / (1000 * 1000 * 1000);
    expected_time.tv_nsec %= 1000 * 1000 * 1000;

    /* the callback time should be greater than or equal to the expected time.*/
    EXPECT_TRUE(
        ((callback_time.tv_sec == expected_time.tv_sec)
                ? (callback_time.tv_nsec >= expected_time.tv_nsec)
                : (callback_time.tv_sec >= expected_time.tv_sec)));

    /* reset for a second run. */
    callback_called = false;

    /* run again. */
    timermode();

    /* a timer is a single-shot timer. */
    EXPECT_FALSE(callback_called);

    /* tear down the loop. */
    timermode_teardown();

    /* clean up. */
    close(lhs);
    close(rhs);
    dispose((disposable_t*)&timer);
}
