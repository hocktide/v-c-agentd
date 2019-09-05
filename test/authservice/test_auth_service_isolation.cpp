/**
 * \file test_authservice_isolation.cpp
 *
 * Isolation tests for the auth service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include "test_auth_service_isolation.h"

using namespace std;

/**
 * Test that we can spawn the auth service.
 */
TEST_F(auth_service_isolation_test, simple_spawn)
{
    ASSERT_EQ(0, auth_service_proc_status);
}
