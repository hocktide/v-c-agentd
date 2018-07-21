/**
 * \file test_parse_commandline_options.cpp
 *
 * Test parsing command-line options.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <gtest/gtest.h>
#include <agentd/commandline.h>
#include <agentd/command.h>
#include <vpr/disposable.h>

using namespace std;

/**
 * \brief Parsing an empty set of command-line options should result in a
 * default bootstrap config.
 */
TEST(parse_commandline_options_test, empty_arguments)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char cmd[] = { 'h', 'e', 'l', 'p', 0 };
    char* empty_args[] = { exename, cmd };

    bootstrap_config_init(&bconf);

    ASSERT_EQ(0,
        parse_commandline_options(
            &bconf, sizeof(empty_args) / sizeof(char*), empty_args));

    /* by default, agentd runs as a daemon. */
    EXPECT_FALSE(bconf.foreground);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief Parsing a -F option should set foreground to true.
 */
TEST(parse_commandline_options_test, foreground_option)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'F', 0 };
    char cmd[] = { 'h', 'e', 'l', 'p', 0 };
    char* args[] = { exename, flags, cmd };

    bootstrap_config_init(&bconf);

    ASSERT_EQ(0,
        parse_commandline_options(
            &bconf, sizeof(args) / sizeof(char*), args));

    /* agentd has been set to run in the foreground. */
    EXPECT_TRUE(bconf.foreground);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief Parsing an invalid options raises an error and prints usage.
 */
TEST(parse_commandline_options_test, invalid_option)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'x', 0 };
    char cmd[] = { 'h', 'e', 'l', 'p', 0 };
    char* args[] = { exename, flags, cmd };

    bootstrap_config_init(&bconf);

    ASSERT_NE(0,
        parse_commandline_options(
            &bconf, sizeof(args) / sizeof(char*), args));

    dispose((disposable_t*)&bconf);
}

/**
 * \brief Parsing an invalid command returns an error.
 */
TEST(parse_commandline_options_test, invalid_command)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char badcmd[] = { 'f', 'o', 'o', 0 };
    char* args[] = { exename, badcmd };

    bootstrap_config_init(&bconf);

    ASSERT_NE(0,
        parse_commandline_options(
            &bconf, sizeof(args) / sizeof(char*), args));

    dispose((disposable_t*)&bconf);
}

/**
 * \brief A command is required.
 */
TEST(parse_commandline_options_test, no_command_fails)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char* args[] = { exename };

    bootstrap_config_init(&bconf);

    ASSERT_NE(0,
        parse_commandline_options(
            &bconf, sizeof(args) / sizeof(char*), args));

    dispose((disposable_t*)&bconf);
}

/**
 * \brief The readconfig command is a valid command.
 */
TEST(parse_commandline_options_test, readconfig_command)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char cmd[] = { 'r', 'e', 'a', 'd', 'c', 'o', 'n', 'f', 'i', 'g', 0 };
    char* args[] = { exename, cmd };

    bootstrap_config_init(&bconf);

    /* precondition: command should be NULL. */
    ASSERT_EQ(nullptr, bconf.command);

    ASSERT_EQ(0,
        parse_commandline_options(
            &bconf, sizeof(args) / sizeof(char*), args));

    /* postcondition: command is set to command_readconfig. */
    ASSERT_EQ(&command_readconfig, bconf.command);

    dispose((disposable_t*)&bconf);
}
