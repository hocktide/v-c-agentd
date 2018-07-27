/**
 * \file test_path_dirname.cpp
 *
 * Test the path_dirname method.
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
 * When an empty string is encountered, return "." to represent the current
 * directory.
 */
TEST(path_dirname, empty_string)
{
    char* dir;

    /* path_dirname should succeed. */
    ASSERT_EQ(0, path_dirname("", &dir));

    /* the dirname string matches what we expect. */
    EXPECT_STREQ(".", dir);

    free(dir);
}

/**
 * When a null string is encountered, return "." to represent the current
 * directory.
 */
TEST(path_dirname, null_path)
{
    char* dir;

    /* path_dirname should succeed. */
    ASSERT_EQ(0, path_dirname(NULL, &dir));

    /* the dirname string matches what we expect. */
    EXPECT_STREQ(".", dir);

    free(dir);
}

/**
 * When a simple filename is encountered, the directory is the current
 * directory.
 */
TEST(path_dirname, simple_filename)
{
    char* dir;

    /* path_dirname should succeed. */
    ASSERT_EQ(0, path_dirname("foo.txt", &dir));

    /* the dirname string matches what we expect. */
    EXPECT_STREQ(".", dir);

    free(dir);
}

/**
 * A filename with a single subdirectory is shortened to the subdir.
 */
TEST(path_dirname, single_subdir)
{
    char* dir;

    /* path_dirname should succeed. */
    ASSERT_EQ(0, path_dirname("build/foo.txt", &dir));

    /* the dirname string matches what we expect. */
    EXPECT_STREQ("build", dir);

    free(dir);
}

/**
 * A filename with multiple subdirs is properly extracted.
 */
TEST(path_dirname, multi_subdir)
{
    char* dir;

    /* path_dirname should succeed. */
    ASSERT_EQ(0, path_dirname("build/host/checked/src/path/foo.txt", &dir));

    /* the dirname string matches what we expect. */
    EXPECT_STREQ("build/host/checked/src/path", dir);

    free(dir);
}

/**
 * An absolute directory is properly extracted.
 */
TEST(path_dirname, multi_subdir_absolute)
{
    char* dir;

    /* path_dirname should succeed. */
    ASSERT_EQ(0, path_dirname("/build/host/checked/src/path/foo.txt", &dir));

    /* the dirname string matches what we expect. */
    EXPECT_STREQ("/build/host/checked/src/path", dir);

    free(dir);
}

/**
 * A filename relative to the current directory is properly handled.
 */
TEST(path_dirname, explicit_curdir)
{
    char* dir;

    /* path_dirname should succeed. */
    ASSERT_EQ(0, path_dirname("./foo.txt", &dir));

    /* the dirname string matches what we expect. */
    EXPECT_STREQ(".", dir);

    free(dir);
}
