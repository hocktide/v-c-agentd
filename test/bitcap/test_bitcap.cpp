/**
 * \file test_bitcap.cpp
 *
 * Test the bitcap system.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <gtest/gtest.h>
#include <agentd/bitcap.h>
#include <vpr/disposable.h>

using namespace std;

/**
 * Test that initializing the bitcap with all falses or all trues works as
 * expected.
 */
TEST(bitcap_test, bitcap_init)
{
    BITCAP(b, 12);

    /* initialize the bitcap to all falses. */
    BITCAP_INIT_FALSE(b);

    /* all bit values are false. */
    EXPECT_FALSE(BITCAP_ISSET(b, 0));
    EXPECT_FALSE(BITCAP_ISSET(b, 1));
    EXPECT_FALSE(BITCAP_ISSET(b, 2));
    EXPECT_FALSE(BITCAP_ISSET(b, 3));
    EXPECT_FALSE(BITCAP_ISSET(b, 4));
    EXPECT_FALSE(BITCAP_ISSET(b, 5));
    EXPECT_FALSE(BITCAP_ISSET(b, 6));
    EXPECT_FALSE(BITCAP_ISSET(b, 7));
    EXPECT_FALSE(BITCAP_ISSET(b, 8));
    EXPECT_FALSE(BITCAP_ISSET(b, 9));
    EXPECT_FALSE(BITCAP_ISSET(b, 10));
    EXPECT_FALSE(BITCAP_ISSET(b, 11));

    /* initialize the bitcap to all trues. */
    BITCAP_INIT_TRUE(b);

    /* all bit values are true. */
    EXPECT_TRUE(BITCAP_ISSET(b, 0));
    EXPECT_TRUE(BITCAP_ISSET(b, 1));
    EXPECT_TRUE(BITCAP_ISSET(b, 2));
    EXPECT_TRUE(BITCAP_ISSET(b, 3));
    EXPECT_TRUE(BITCAP_ISSET(b, 4));
    EXPECT_TRUE(BITCAP_ISSET(b, 5));
    EXPECT_TRUE(BITCAP_ISSET(b, 6));
    EXPECT_TRUE(BITCAP_ISSET(b, 7));
    EXPECT_TRUE(BITCAP_ISSET(b, 8));
    EXPECT_TRUE(BITCAP_ISSET(b, 9));
    EXPECT_TRUE(BITCAP_ISSET(b, 10));
    EXPECT_TRUE(BITCAP_ISSET(b, 11));
}

/**
 * Test that setting a bit to true or false works as expected.
 */
TEST(bitcap_test, bitcap_set)
{
    BITCAP(b, 12);

    /* set the bitcap to all falses. */
    BITCAP_INIT_FALSE(b);

    /* bit value 7 is false. */
    ASSERT_FALSE(BITCAP_ISSET(b, 7));

    /* set bit value 7 to true. */
    BITCAP_SET_TRUE(b, 7);

    /* bit value 7 is true. */
    ASSERT_TRUE(BITCAP_ISSET(b, 7));

    /* set bit value 7 to false. */
    BITCAP_SET_FALSE(b, 7);

    /* bit value 7 is false. */
    ASSERT_FALSE(BITCAP_ISSET(b, 7));
}

/**
 * Test that we can form an intersection of two bitcaps.
 */
TEST(bitcap_test, bitcap_intersect)
{
    BITCAP(b, 12);
    BITCAP(c, 12);
    BITCAP(d, 12);

    /* set bitcaps b, c, and d to all falses. */
    BITCAP_INIT_FALSE(b);
    BITCAP_INIT_FALSE(c);
    BITCAP_INIT_FALSE(d);

    /* b bit value 7 is true. */
    BITCAP_SET_TRUE(b, 7);

    /* c bit values 7 and 8 are true. */
    BITCAP_SET_TRUE(c, 7);
    BITCAP_SET_TRUE(c, 8);

    /* set d to the intersection of b and c. */
    BITCAP_INTERSECT(d, b, c);

    /* bit value 7 is true. */
    ASSERT_TRUE(BITCAP_ISSET(d, 7));
    /* bit value 8 is false. */
    ASSERT_FALSE(BITCAP_ISSET(d, 8));
}

/**
 * Test that we can form a union of two bitcaps.
 */
TEST(bitcap_test, bitcap_union)
{
    BITCAP(b, 12);
    BITCAP(c, 12);
    BITCAP(d, 12);

    /* set bitcaps b, c, and d to all falses. */
    BITCAP_INIT_FALSE(b);
    BITCAP_INIT_FALSE(c);
    BITCAP_INIT_FALSE(d);

    /* b bit value 6 is true. */
    BITCAP_SET_TRUE(b, 6);

    /* c bit values 7 and 8 are true. */
    BITCAP_SET_TRUE(c, 7);
    BITCAP_SET_TRUE(c, 8);

    /* set d to the intersection of b and c. */
    BITCAP_UNION(d, b, c);

    /* bit value 6 is true. */
    ASSERT_TRUE(BITCAP_ISSET(d, 6));
    /* bit value 7 is true. */
    ASSERT_TRUE(BITCAP_ISSET(d, 7));
    /* bit value 8 is true. */
    ASSERT_TRUE(BITCAP_ISSET(d, 8));
}
