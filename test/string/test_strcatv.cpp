/**
 * \file test_strcatv.cpp
 *
 * Test variable strcat.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <gtest/gtest.h>
#include <agentd/string.h>

using namespace std;

/**
 * \brief Passing NULL as the first argument of strcatv returns an empty string.
 */
TEST(strcatv, param_null)
{
    char* str = strcatv(nullptr);
    ASSERT_NE(nullptr, str);
    EXPECT_STREQ("", str);
    free(str);
}

/**
 * \brief We can concatenate one string and null.
 */
TEST(strcatv, one_param)
{
    char* str = strcatv("foo", nullptr);
    ASSERT_NE(nullptr, str);
    EXPECT_STREQ("foo", str);
    free(str);
}

/**
 * \brief We can concatenate two strings and null.
 */
TEST(strcatv, two_params)
{
    char* str = strcatv("foo", "bar", nullptr);
    ASSERT_NE(nullptr, str);
    EXPECT_STREQ("foobar", str);
    free(str);
}

/**
 * \brief We can concatenate many strings.
 */
TEST(strcatv, many_params)
{
    char* str = strcatv("f", "o", "o", "", "b", "a", "r", "!", nullptr);
    ASSERT_NE(nullptr, str);
    EXPECT_STREQ("foobar!", str);
    free(str);
}
