/**
 * \file test_consensusservice_isolation_helpers.cpp
 *
 * Helpers for the consensus service isolation test.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/consensusservice.h>
#include <agentd/status_codes.h>
#include <vpr/allocator/malloc_allocator.h>

#include "test_consensusservice_isolation.h"

using namespace std;

const uint32_t
    consensusservice_isolation_test::EXPECTED_CHILD_INDEX = 19U;

void consensusservice_isolation_test::SetUp()
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

    /* create the control socket pair for the consensus service. */
    int controlsock_srv;
    ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &controlsock, &controlsock_srv);

    /* create the bootstrap config. */
    bootstrap_config_init(&bconf);

    /* set the default config. */
    memset(&conf, 0, sizeof(conf));
    conf.hdr.dispose = &config_dispose;

    /* spawn the consensus service process. */
    consensus_proc_status =
        start_consensus_proc(
            &bconf, &conf, logsock, datasock_srv, controlsock_srv,
            &consensuspid, false);

    /* create the mock dataservice. */
    dataservice = make_unique<mock_dataservice::mock_dataservice>(datasock);
}

void consensusservice_isolation_test::TearDown()
{
    /* terminate the consensus service process. */
    if (0 == consensus_proc_status)
    {
        int status = 0;
        close(controlsock);
        kill(consensuspid, SIGTERM);
        waitpid(consensuspid, &status, 0);
    }

    /* set the old path. */
    setenv("PATH", oldpath, 1);

    /* clean up. */
    dataservice->stop();
    dispose((disposable_t*)&conf);
    dispose((disposable_t*)&bconf);
    close(logsock);
    close(rlogsock);
    close(datasock);
    free(path);
    if (suite_instance_initialized)
    {
        dispose((disposable_t*)&suite);
    }
    dispose((disposable_t*)&alloc_opts);
}

int consensusservice_isolation_test::dataservice_mock_register_helper()
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

int consensusservice_isolation_test::
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

int consensusservice_isolation_test::
    dataservice_mock_valid_connection_teardown()
{
    /* the child index should have been closed. */
    if (!dataservice->request_matches_child_context_close(EXPECTED_CHILD_INDEX))
        return 1;

    return 0;
}
