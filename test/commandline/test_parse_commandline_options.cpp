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

    parse_commandline_options(
        &bconf, sizeof(empty_args) / sizeof(char*), empty_args);

    /* by default, agentd runs as a daemon. */
    EXPECT_FALSE(bconf.foreground);
    /* the help command is set. */
    EXPECT_EQ(&command_help, bconf.command);

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

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* agentd has been set to run in the foreground. */
    EXPECT_TRUE(bconf.foreground);
    /* the help command is set. */
    EXPECT_EQ(&command_help, bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief Parsing a -c config should set the config file name.
 */
TEST(parse_commandline_options_test, config_option_space)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'c', 0 };
    char config[] = { 'o', 't', 'h', 'e', 'r', '.', 'c', 'o', 'n', 'f', 0 };
    char cmd[] = { 'h', 'e', 'l', 'p', 0 };
    char* args[] = { exename, flags, config, cmd };

    bootstrap_config_init(&bconf);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* agentd has its config file overridden. */
    EXPECT_STREQ("other.conf", bconf.config_file);
    /* the help command is set. */
    EXPECT_EQ(&command_help, bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief Parsing a -c config should set the config file name (no space).
 */
TEST(parse_commandline_options_test, config_option_no_space)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'c', 'o', 't', 'h', 'e', 'r', '.', 'c', 'o', 'n',
        'f', 0 };
    char cmd[] = { 'h', 'e', 'l', 'p', 0 };
    char* args[] = { exename, flags, cmd };

    bootstrap_config_init(&bconf);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* agentd has its config file overridden. */
    EXPECT_STREQ("other.conf", bconf.config_file);
    /* the help command is set. */
    EXPECT_EQ(&command_help, bconf.command);

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

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* the error_usage command is set. */
    EXPECT_EQ(&command_error_usage, bconf.command);

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

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* the error_usage command is set. */
    EXPECT_EQ(&command_error_usage, bconf.command);

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

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* the error_usage command is set. */
    EXPECT_EQ(&command_error_usage, bconf.command);

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

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* postcondition: command is set to command_readconfig. */
    ASSERT_EQ(&command_readconfig, bconf.command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief The readconfig private command is a valid private command.
 */
TEST(parse_commandline_options_test, readconfig_private_command)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'P', 0 };
    char cmd[] = { 'r', 'e', 'a', 'd', 'c', 'o', 'n', 'f', 'i', 'g', 0 };
    char* args[] = { exename, flags, cmd };

    bootstrap_config_init(&bconf);

    /* precondition: command should be NULL. */
    ASSERT_EQ(nullptr, bconf.command);
    /* precondition: private_command should be NULL. */
    ASSERT_EQ(nullptr, bconf.private_command);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* postcondition: command is set to NULL. */
    ASSERT_EQ(nullptr, bconf.command);
    /* postcondition: private command is set to private_command_readconfig. */
    ASSERT_EQ(&private_command_readconfig, bconf.private_command);

    dispose((disposable_t*)&bconf);
}

/**
 * \brief An invalid private command calls error_usage.
 */
TEST(parse_commandline_options_test, readconfig_invalid_private_command)
{
    bootstrap_config_t bconf;
    char exename[] = { 'a', 'g', 'e', 'n', 't', 'd', 0 };
    char flags[] = { '-', 'P', 0 };
    char cmd[] = { 'f', 'o', 'o', 0 };
    char* args[] = { exename, flags, cmd };

    bootstrap_config_init(&bconf);

    /* precondition: command should be NULL. */
    ASSERT_EQ(nullptr, bconf.command);
    /* precondition: private_command should be NULL. */
    ASSERT_EQ(nullptr, bconf.private_command);

    parse_commandline_options(
        &bconf, sizeof(args) / sizeof(char*), args);

    /* postcondition: command is set to command_error_usage. */
    ASSERT_EQ(&command_error_usage, bconf.command);
    /* postcondition: private command is NULL. */
    ASSERT_EQ(nullptr, bconf.private_command);

    dispose((disposable_t*)&bconf);
}
