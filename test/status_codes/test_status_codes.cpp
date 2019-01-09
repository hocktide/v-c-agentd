/**
 * \file test_status_codes.cpp
 *
 * Test that status codes work as expected.
 *
 * \copyright 2018-2019 Velo-Payments, Inc.  All rights reserved.
 */

#include <gtest/gtest.h>
#include <agentd/status_codes.h>

using namespace std;

/**
 * Test that status codes are constructed properly.
 */
TEST(status_codes_test, basic_test)
{
    EXPECT_EQ(0x8000001U, AGENTD_ERROR_GENERAL_OUT_OF_MEMORY);
}
