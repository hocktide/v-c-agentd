/**
 * \file directory_test_helper.h
 *
 * Helper class for managing building unique test directory names.
 *
 * \copyright 2018 Velo-Payments, Inc.  All rights reserved.
 */

#ifndef TEST_DIRECTORY_TEST_HELPER_HEADER_GUARD
#define TEST_DIRECTORY_TEST_HELPER_HEADER_GUARD

#include <cstdint>
#include <string>
#include <vccrypt/block_cipher.h>
#include <vpr/allocator.h>

/* this header will only work for C++. */
#if !defined(__cplusplus)
#error This is a C++ header file.
#endif /*! defined(__cplusplus)*/

class directory_test_helper {
protected:
    void SetUp(const uint8_t* _dir_key, const char* dir_path);
    void TearDown();

    int setDirectoryName(std::uint64_t offset, std::string& dname);
    int createDirectoryName(std::uint64_t arg, std::string& dname);

    allocator_options_t dircrypt_alloc_opts;
    vccrypt_block_options_t dircrypt_options;
    char* dbDirPath;
    int dircrypt_options_init_result;
    static const uint8_t zero_uuid[16];

private:
    uint8_t dir_key[32];
};

#endif /*TEST_DIRECTORY_TEST_HELPER_HEADER_GUARD*/
