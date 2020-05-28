/**
 * \file test_canonizationservice_isolation_helpers.cpp
 *
 * Helpers for the canonization service isolation test.
 *
 * \copyright 2019-2020 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/randomservice.h>
#include <agentd/status_codes.h>
#include <vpr/allocator/malloc_allocator.h>

#include "test_canonizationservice_isolation.h"

using namespace std;

const uint32_t
    canonizationservice_isolation_test::EXPECTED_CHILD_INDEX = 19U;

void canonizationservice_isolation_test::SetUp()
{
    vccrypt_suite_register_velo_v1();

    /* initialize allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* initialize the crypto suite. */
    if (VCCRYPT_STATUS_SUCCESS ==
        vccrypt_suite_options_init(
            &suite, &alloc_opts, VCCRYPT_SUITE_VELO_V1))
    {
        suite_instance_initialized = true;
    }
    else
    {
        suite_instance_initialized = false;
    }

    /* set the path for running agentd. */
    getcwd(wd, sizeof(wd));
    oldpath = getenv("PATH");
    if (NULL != oldpath)
    {
        path =
            strcatv(wd, "/build/host/release/bin", ":", oldpath, NULL);
    }
    else
    {
        path = strcatv(wd, "/build/host/release/bin");
    }

    setenv("PATH", path, 1);

    /* log to standard error. */
    logsock = dup(STDERR_FILENO);
    rlogsock = dup(STDERR_FILENO);

    /* create the socket pair for the datasock. */
    int datasock_srv;
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &datasock, &datasock_srv);

    /* create the control socket pair for the canonization service. */
    int controlsock_srv;
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &controlsock, &controlsock_srv);

    /* create the bootstrap config. */
    bootstrap_config_init(&bconf);

    /* set the default config. */
    memset(&conf, 0, sizeof(conf));
    conf.hdr.dispose = &config_dispose;

    /* spawn the random service process. */
    random_proc_status =
        randomservice_proc(
            &bconf, &conf, &rlogsock, &rprotosock, &randompid, false);

    /* spawn the canonization service process. */
    canonization_proc_status =
        start_canonization_proc(
            &bconf, &conf, &logsock, &datasock_srv, &rprotosock,
            &controlsock_srv, &canonizationpid, false);

    /* create the mock dataservice. */
    dataservice = make_unique<mock_dataservice::mock_dataservice>(datasock);
}

void canonizationservice_isolation_test::TearDown()
{
    /* terminate the random service. */
    if (0 == random_proc_status)
    {
        int status = 0;
        kill(randompid, SIGTERM);
        waitpid(randompid, &status, 0);
    }

    /* terminate the canonization service process. */
    if (0 == canonization_proc_status)
    {
        int status = 0;
        close(controlsock);
        kill(canonizationpid, SIGTERM);
        waitpid(canonizationpid, &status, 0);
    }

    /* set the old path. */
    setenv("PATH", oldpath, 1);

    /* clean up. */
    dataservice->stop();
    dispose((disposable_t*)&conf);
    dispose((disposable_t*)&bconf);
    if (logsock >= 0)
        close(logsock);
    if (rlogsock >= 0)
        close(rlogsock);
    close(datasock);
    free(path);
    if (suite_instance_initialized)
    {
        dispose((disposable_t*)&suite);
    }
    dispose((disposable_t*)&alloc_opts);
}

int canonizationservice_isolation_test::dataservice_mock_register_helper()
{
    /* mock the child context create call. */
    dataservice->register_callback_child_context_create(
        [&](const dataservice_request_child_context_create_t&,
            std::ostream& payout) {
            void* payload = nullptr;
            size_t payload_size = 0U;

            int retval =
                dataservice_encode_response_child_context_create(
                    &payload, &payload_size, EXPECTED_CHILD_INDEX);
            if (AGENTD_STATUS_SUCCESS != retval)
                return retval;

            /* make sure to clean up memory when we fall out of scope. */
            unique_ptr<void, decltype(free)*> cleanup(payload, &free);

            /* write the payload. */
            payout.write((const char*)payload, payload_size);

            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    /* mock the child context close call. */
    dataservice->register_callback_child_context_close(
        [&](const dataservice_request_child_context_close_t&,
            std::ostream&) {
            /* success. */
            return AGENTD_STATUS_SUCCESS;
        });

    return 0;
}

int canonizationservice_isolation_test::
    dataservice_mock_valid_connection_setup()
{
    /* a child context should have been created. */
    BITCAP(testbits, DATASERVICE_API_CAP_BITS_MAX);
    BITCAP_INIT_FALSE(testbits);
    /* TODO - add valid bits here. */
    if (!dataservice->request_matches_child_context_create(testbits))
        return 1;

    return 0;
}

int canonizationservice_isolation_test::
    dataservice_mock_valid_connection_teardown()
{
    /* the child index should have been closed. */
    if (!dataservice->request_matches_child_context_close(EXPECTED_CHILD_INDEX))
        return 1;

    return 0;
}

int canonizationservice_isolation_test::
    canonizationservice_configure_and_start(int max_milliseconds, int max_txns)
{
    int retval;
    uint32_t status, offset;
    agent_config_t conf;

    /* set config values for canonization service. */
    conf.block_max_milliseconds_set = true;
    conf.block_max_milliseconds = max_milliseconds;
    conf.block_max_transactions_set = true;
    conf.block_max_transactions = max_txns;

    /* send the configure service request. */
    retval =
        canonization_api_sendreq_configure(controlsock, &conf);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* receive the configure service response. */
    retval =
        canonization_api_recvresp_configure(controlsock, &offset, &status);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* verify that the configure request was successful. */
    retval = (int)status;
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* send the start request. */
    retval = canonization_api_sendreq_start(controlsock);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* receive the start response. */
    retval = canonization_api_recvresp_start(controlsock, &offset, &status);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* verify that the start request was successful. */
    retval = (int)status;
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        return retval;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
