/**
 * \file test_bootstrap_config.cpp
 *
 * Test the boostrap configuration functions.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <gtest/gtest.h>
#include <agentd/bootstrap_config.h>
#include <vpr/disposable.h>

using namespace std;

/**
 * \brief Initializing the bootstrap config structure empties all values.
 */
TEST(bootstrap_config_test, bootstrap_config_init)
{
    bootstrap_config_t bconf;

    bootstrap_config_init(&bconf);

    /* by default, agentd runs as a daemon. */
    EXPECT_FALSE(bconf.foreground);
    /* by default, command is NULL. */
    EXPECT_EQ(nullptr, bconf.command);
    /* by default, private_command is NULL. */
    EXPECT_EQ(nullptr, bconf.private_command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief bootstrap_config_set_foreground sets the foreground field.
 */
TEST(bootstrap_config_test, bootstrap_config_set_foreground)
{
    bootstrap_config_t bconf;

    bootstrap_config_init(&bconf);

    /* Precondition: foreground is false. */
    ASSERT_FALSE(bconf.foreground);

    /* run bootstrap_config_set_foreground. */
    bootstrap_config_set_foreground(&bconf, true);

    /* Postcondition: foreground is true. */
    EXPECT_TRUE(bconf.foreground);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief bootstrap_config_set_command sets the command field.
 */
TEST(bootstrap_config_test, bootstrap_config_set_command)
{
    bootstrap_config_t bconf;

    bootstrap_config_init(&bconf);

    /* Precondition: command is NULL. */
    ASSERT_EQ(nullptr, bconf.command);

    /* run bootstrap_config_set_command. */
    bootstrap_config_set_command(&bconf, (bootstrap_config_command_t)0x1234);

    /* Postcondition: command is set. */
    EXPECT_EQ((bootstrap_config_command_t)0x1234, bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief bootstrap_config_set_private_command sets the private_command field.
 */
TEST(bootstrap_config_test, bootstrap_config_set_private_command)
{
    bootstrap_config_t bconf;

    bootstrap_config_init(&bconf);

    /* Precondition: private_command is NULL. */
    ASSERT_EQ(nullptr, bconf.private_command);

    /* run bootstrap_config_set_private_command. */
    bootstrap_config_set_private_command(
        &bconf, (bootstrap_config_private_command_t)0x1234);

    /* Postcondition: private_command is set. */
    EXPECT_EQ(
        (bootstrap_config_private_command_t)0x1234, bconf.private_command);

    dispose((disposable_t*)&bconf);
}
