/**
 * \file test_path_resolve.cpp
 *
 * Test the path_resolve method.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#include <gtest/gtest.h>
#include <agentd/commandline.h>
#include <agentd/command.h>
#include <agentd/path.h>
#include <limits.h>
#include <string>
#include <vpr/disposable.h>

using namespace std;

/**
 * \brief It is not possible to resolve a non-existent binary from an empty
 * path.
 */
TEST(path_resolve, empty_path_no_local)
{
    char* resolved = nullptr;

    ASSERT_NE(0, path_resolve("foosh", "", &resolved));
}

/**
 * \brief It is possible to resolve a binary from a simple path.
 */
TEST(path_resolve, simple_path)
{
    char* resolved = nullptr;

    ASSERT_EQ(0, path_resolve("cat", "/bin", &resolved));

    EXPECT_STREQ("/bin/cat", resolved);

    free(resolved);
}

/**
 * \brief A non-existent binary and a simple path do not resolve.
 */
TEST(path_resolve, simple_path_non_existent_binary)
{
    char* resolved = nullptr;

    ASSERT_NE(0, path_resolve("foosh", "/bin", &resolved));
}

/**
 * \brief It is possible to resolve a binary from a multi path.
 */
TEST(path_resolve, multi_path)
{
    char* resolved = nullptr;

    ASSERT_EQ(0,
        path_resolve("cat", "/etasuetheoasu:/teasuthoseu:/bin", &resolved));

    EXPECT_STREQ("/bin/cat", resolved);

    free(resolved);
}

/**
 * \brief A non-existent binary and a multi path do not resolve.
 */
TEST(path_resolve, multi_path_non_existent_binary)
{
    char* resolved = nullptr;

    ASSERT_NE(0,
        path_resolve("foosh", "/etasuetheoasu:/teasuthoseu:/bin", &resolved));
}

/**
 * \brief If a binary is an absolute path but it does not exist, then
 * path_resolve fails.
 */
TEST(path_resolve, nonexistent_absolute_path)
{
    char* resolved = nullptr;

    ASSERT_NE(0, path_resolve("/bin/fooshsthsthsth", "", &resolved));
}

/**
 * \brief If a binary is an absolute path and it exists, then resolved is
 * updated to the canonical path for this value and path_resolve succeeds.
 */
TEST(path_resolve, canonical_absolute_path)
{
    char* resolved = nullptr;

    ASSERT_EQ(0, path_resolve("/bin//cat", "", &resolved));

    EXPECT_STREQ("/bin/cat", resolved);

    free(resolved);
}

/**
 * \brief If a relative path starting with "." is encountered, attempt to
 * canonicalize it.  If it cannot be resolved, fail.
 */
TEST(path_resolve, canonical_relative_path_fail)
{
    char* resolved = nullptr;

    ASSERT_NE(0, path_resolve("./bin//cat", "", &resolved));
}

/**
 * \brief If a relative path starting with "." is encountered, attempt to
 * canonicalize it.  If it can be resolved and is executable, succeed.
 */
TEST(path_resolve, canonical_relative_path)
{
    char* resolved = nullptr;
    char* pwd = getcwd(nullptr, PATH_MAX);
    string expected_resolved = string(pwd) + "/build/host/checked/bin/agentd";

    ASSERT_EQ(0,
        path_resolve("./build/host/checked/bin/agentd", "", &resolved));

    EXPECT_STREQ(expected_resolved.c_str(), resolved);

    free(pwd);
    free(resolved);
}
