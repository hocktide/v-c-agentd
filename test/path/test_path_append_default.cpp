/**
 * \file test_path_append_default.cpp
 *
 * Test the path_append_default method.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <gtest/gtest.h>
#include <agentd/commandline.h>
#include <agentd/command.h>
#include <agentd/path.h>
#include <paths.h>
#include <string>
#include <vpr/disposable.h>

using namespace std;

/**
 * \brief Append the default path onto an empty string.
 */
TEST(path_append_default, empty_string)
{
    char* outpath = nullptr;

    ASSERT_EQ(0, path_append_default("", &outpath));

    EXPECT_STREQ(_PATH_DEFPATH, outpath);

    free(outpath);
}

/**
 * \brief Append the default path onto an arbitrary path
 */
TEST(path_append_default, arbitrary_path_1)
{
    char* outpath = nullptr;
    string arbitrary_path = string("baz") + string(":") + _PATH_DEFPATH;

    ASSERT_EQ(0, path_append_default("baz", &outpath));

    EXPECT_STREQ(arbitrary_path.c_str(), outpath);

    free(outpath);
}

/**
 * \brief Append the default path onto an arbitrary path
 */
TEST(path_append_default, arbitrary_path_2)
{
    char* outpath = nullptr;
    string begin_path("/bin:/usr/bin:/home/foo/bin");
    string arbitrary_path =
        string("/bin:/usr/bin:/home/foo/bin") + string(":") + _PATH_DEFPATH;

    ASSERT_EQ(0, path_append_default(begin_path.c_str(), &outpath));

    EXPECT_STREQ(arbitrary_path.c_str(), outpath);

    free(outpath);
}
