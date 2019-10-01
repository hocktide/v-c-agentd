/**
 * \file mocks/dataservice.cpp
 *
 * Mock dataservice methods.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>

#include "dataservice.h"

using namespace mock_dataservice;
using namespace std;

/**
 * \brief Create a mock dataservice instance that will listen on the
 * given socket when started.
 *
 * \param _datasock         The socket used to listen for dataservice
 *                          requests.
 */
mock_dataservice::mock_dataservice::mock_dataservice(int _datasock)
    : datasock(_datasock)
    , running(false)
    , testsock(-1)
    , mocksock(-1)
{
}

/**
 * \brief Make sure to stop the mock dataservice if running on
 * destruction.
 */
mock_dataservice::mock_dataservice::~mock_dataservice()
{
    if (-1 != datasock)
        close(datasock);

    if (-1 != mocksock)
        close(mocksock);

    if (-1 != testsock)
        close(testsock);

    stop();
}

/**
 * \brief Start the mock dataservice with the current mock settings.
 */
void mock_dataservice::mock_dataservice::start()
{
    /* only start the mock dataservice once. */
    if (running)
    {
        return;
    }

    /* set up the socketpair for running the test service. */
    if (AGENTD_STATUS_SUCCESS !=
        ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &testsock, &mocksock))
    {
        return;
    }

    /* fork the process. */
    mock_pid = fork();
    if (mock_pid < 0)
    {
        goto cleanup_socketpair;
    }

    /* child */
    if (0 == mock_pid)
    {
        close(testsock);
        testsock = -1;
        mock_process();
        exit(0);
    }
    /* parent */
    else
    {
        close(datasock);
        datasock = -1;
        close(mocksock);
        mocksock = -1;
        running = true;
        return;
    }

cleanup_socketpair:
    close(testsock);
    testsock = -1;
    close(mocksock);
    mocksock = -1;
}

/**
 * \brief Stop the mock dataservice if running.
 */
void mock_dataservice::mock_dataservice::stop()
{
    /* only stop the mock dataservice if running. */
    if (!running)
        return;

    /* sleep for a sec to propagate the close. */
    usleep(10000);

    /* kill the child process. */
    kill(mock_pid, SIGTERM);

    /* wait on the pid to terminate.*/
    int wstatus = 0;
    waitpid(mock_pid, &wstatus, 0);

    /* We are no longer running. */
    running = false;
}

/**
 * \brief Register a mock callback for payload_artifact_read.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_payload_artifact_read(
    function<
        int(const dataservice_request_payload_artifact_read_t&,
            ostream&)>
        cb)
{
    payload_artifact_read_callback = cb;
}

/**
 * \brief Register a mock callback for block_id_by_height_read.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_block_id_by_height_read(
    function<
        int(const dataservice_request_block_id_by_height_read_t&,
            ostream&)>
        cb)
{
    block_id_by_height_read_callback = cb;
}

/**
 * \brief Register a mock callback for block_id_latest_read.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_block_id_latest_read(
    function<
        int(const dataservice_request_block_id_latest_read_t&,
            ostream&)>
        cb)
{
    block_id_latest_read_callback = cb;
}

/**
 * \brief Register a mock callback for block_make.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_block_make(
    function<
        int(const dataservice_request_block_make_t&,
            ostream&)>
        cb)
{
    block_make_callback = cb;
}

/**
 * \brief Register a mock callback for block_read.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_block_read(
    function<
        int(const dataservice_request_block_read_t&,
            ostream&)>
        cb)
{
    block_read_callback = cb;
}

/**
 * \brief Register a mock callback for canonized_transaction_get.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_canonized_transaction_get(
    function<
        int(const dataservice_request_canonized_transaction_get_t&,
            ostream&)>
        cb)
{
    canonized_transaction_get_callback = cb;
}

/**
 * \brief Register a mock callback for child_context_close.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_child_context_close(
    function<
        int(const dataservice_request_child_context_close_t&,
            ostream&)>
        cb)
{
    child_context_close_callback = cb;
}

/**
 * \brief Register a mock callback for child_context_create.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_child_context_create(
    function<
        int(const dataservice_request_child_context_create_t&,
            ostream&)>
        cb)
{
    child_context_create_callback = cb;
}

/**
 * \brief Register a mock callback for global_setting_get.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_global_setting_get(
    function<
        int(const dataservice_request_global_setting_get_t&,
            ostream&)>
        cb)
{
    global_setting_get_callback = cb;
}

/**
 * \brief Register a mock callback for global_setting_set.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_global_setting_set(
    function<
        int(const dataservice_request_global_setting_set_t&,
            ostream&)>
        cb)
{
    global_setting_set_callback = cb;
}

/**
 * \brief Register a mock callback for transaction_drop.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_transaction_drop(
    function<
        int(const dataservice_request_transaction_drop_t&,
            ostream&)>
        cb)
{
    transaction_drop_callback = cb;
}

/**
 * \brief Register a mock callback for transaction_get.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_transaction_get(
    function<
        int(const dataservice_request_transaction_get_t&,
            ostream&)>
        cb)
{
    transaction_get_callback = cb;
}

/**
 * \brief Register a mock callback for transaction_get_first.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_transaction_get_first(
    function<
        int(const dataservice_request_transaction_get_first_t&,
            ostream&)>
        cb)
{
    transaction_get_first_callback = cb;
}

/**
 * \brief Register a mock callback for transaction_submit.
 *
 * \param cb                The callback to register.
 */
void mock_dataservice::mock_dataservice::register_callback_transaction_submit(
    function<
        int(const dataservice_request_transaction_submit_t&,
            ostream&)>
        cb)
{
    transaction_submit_callback = cb;
}

/**
 * \brief Run the mock dataservice process.
 *
 * Read request packets from the process and write canned response
 * packets, possibly using the mock override callbacks.
 */
void mock_dataservice::mock_dataservice::mock_process()
{
    /* read every transaction on the socket. */
    while (mock_read_and_dispatch())
        ;

    /* close the socket. */
    close(datasock);
    datasock = -1;
}

/**
 * \brief Read and dispatch one request.
 *
 * \returns true if a request was read, and false if anything goes
 *          wrong (e.g. a socket was closed).
 */
bool mock_dataservice::mock_dataservice::mock_read_and_dispatch()
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    uint32_t nmethod = 0U;
    uint32_t method = 0U;
    const uint8_t* breq = nullptr;
    size_t payload_size = 0U;

    /* read a request to the data service mock. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(datasock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* immediately write this request to the mocksock to log it. */
    if (AGENTD_STATUS_SUCCESS != ipc_write_data_block(mocksock, val, size))
    {
        retval = false;
        goto cleanup_val;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* derive the payload size. */
    payload_size = size - sizeof(uint32_t);

    /* decode the method. */
    switch (method)
    {
        /* handle root context create method. */
        case DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_CREATE:
            retval =
                mock_decode_and_dispatch_root_context_create(
                    breq, payload_size);
            break;

        /* handle root context reduce capabilites. */
        case DATASERVICE_API_METHOD_LL_ROOT_CONTEXT_REDUCE_CAPS:
            retval =
                mock_decode_and_dispatch_root_context_reduce_caps(
                    breq, payload_size);
            break;

        /* handle child context create call. */
        case DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE:
            retval =
                mock_decode_and_dispatch_child_context_create(
                    breq, payload_size);
            break;

        /* handle child context close call. */
        case DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE:
            retval =
                mock_decode_and_dispatch_child_context_close(
                    breq, payload_size);
            break;

        /* handle global settings get call. */
        case DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_READ:
            retval =
                mock_decode_and_dispatch_global_setting_get(
                    breq, payload_size);
            break;

        /* handle global settings set call. */
        case DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE:
            retval =
                mock_decode_and_dispatch_global_setting_set(
                    breq, payload_size);
            break;

        /* handle transaction submit. */
        case DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT:
            retval =
                mock_decode_and_dispatch_transaction_submit(
                    breq, payload_size);
            break;

        /* handle transaction get first. */
        case DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_FIRST_READ:
            retval =
                mock_decode_and_dispatch_transaction_get_first(
                    breq, payload_size);
            break;

        /* handle transaction get. */
        case DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ:
            retval =
                mock_decode_and_dispatch_transaction_get(
                    breq, payload_size);
            break;

        /* handle transaction drop. */
        case DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_DROP:
            retval =
                mock_decode_and_dispatch_transaction_drop(
                    breq, payload_size);
            break;

        /* handle artifact read. */
        case DATASERVICE_API_METHOD_APP_ARTIFACT_READ:
            retval =
                mock_decode_and_dispatch_artifact_read(
                    breq, payload_size);
            break;

        /* handle block make. */
        case DATASERVICE_API_METHOD_APP_BLOCK_WRITE:
            retval =
                mock_decode_and_dispatch_block_make(
                    breq, payload_size);
            break;

        /* handle block read. */
        case DATASERVICE_API_METHOD_APP_BLOCK_READ:
            retval =
                mock_decode_and_dispatch_block_read(
                    breq, payload_size);
            break;

        /* handle block by height read. */
        case DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ:
            retval =
                mock_decode_and_dispatch_block_id_by_height_read(
                    breq, payload_size);
            break;

        /* handle latest block ID read. */
        case DATASERVICE_API_METHOD_APP_BLOCK_ID_LATEST_READ:
            retval =
                mock_decode_and_dispatch_block_id_latest_read(
                    breq, payload_size);
            break;

        /* handle canonized transaction read. */
        case DATASERVICE_API_METHOD_APP_TRANSACTION_READ:
            retval =
                mock_decode_and_dispatch_canonized_transaction_get(
                    breq, payload_size);
            break;

        default:
            mock_write_status(
                method, 0U, AGENTD_ERROR_DATASERVICE_REQUEST_PACKET_BAD,
                nullptr, 0);
            retval = false;
            break;
    }

    /* done */
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Write the status back to the caller.
 *
 * \param method    The method requested.
 * \param offset    The child offset.
 * \param status    The status code.
 * \param payload   The response payload, or ptr if none.
 * \param size      The response payload size.
 */
void mock_dataservice::mock_dataservice::mock_write_status(
    uint32_t method, uint32_t offset, uint32_t status,
    const void* payload, size_t size)
{
    stringstream out;

    uint32_t net_method = htonl(method);
    uint32_t net_offset = htonl(offset);
    uint32_t net_status = htonl(status);

    out.write((const char*)&net_method, sizeof(net_method));
    out.write((const char*)&net_offset, sizeof(net_offset));
    out.write((const char*)&net_status, sizeof(net_status));
    if (nullptr != payload)
        out.write((const char*)payload, size);

    std::string output = out.str();

    ipc_write_data_block(datasock, output.data(), output.size());
}

/**
 * \brief Mock for the root context create call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_root_context_create(
        const void* request, size_t payload_size)
{
    (void)request;
    (void)payload_size;
    return false;
}

/**
 * \brief Mock for the root capabilities call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_root_context_reduce_caps(
        const void* request, size_t payload_size)
{
    (void)request;
    (void)payload_size;
    return false;
}

/**
 * \brief Mock for the child context create call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_child_context_create(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_child_context_create_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_child_context_create(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!child_context_create_callback)
    {
        status = child_context_create_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the child context close call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_child_context_close(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_child_context_close_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_child_context_close(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!child_context_close_callback)
    {
        status = child_context_close_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the global setting get call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_global_setting_get(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_global_setting_get_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_global_setting_get(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!global_setting_get_callback)
    {
        status = global_setting_get_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_READ, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the global setting set call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_global_setting_set(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_global_setting_set_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_global_setting_set(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!global_setting_set_callback)
    {
        status = global_setting_set_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the transaction submit call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_transaction_submit(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_transaction_submit_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_transaction_submit(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!transaction_submit_callback)
    {
        status = transaction_submit_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the transaction get first call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_transaction_get_first(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_transaction_get_first_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_transaction_get_first(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!transaction_get_first_callback)
    {
        status = transaction_get_first_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_FIRST_READ, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the transaction get call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_transaction_get(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_transaction_get_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_transaction_get(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!transaction_get_callback)
    {
        status = transaction_get_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the transaction drop call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_transaction_drop(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_transaction_drop_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_transaction_drop(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!transaction_drop_callback)
    {
        status = transaction_drop_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_DROP, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the artifact read call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_artifact_read(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_payload_artifact_read_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_payload_artifact_read(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!payload_artifact_read_callback)
    {
        status = payload_artifact_read_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_APP_ARTIFACT_READ, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the block make call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_block_make(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_block_make_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_block_make(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!block_make_callback)
    {
        status = block_make_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_APP_BLOCK_WRITE, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the block read call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_block_read(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_block_read_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_block_read(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!block_read_callback)
    {
        status = block_read_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_APP_BLOCK_READ, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the block id by height read call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_block_id_by_height_read(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_block_id_by_height_read_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_block_id_by_height_read(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!block_id_by_height_read_callback)
    {
        status = block_id_by_height_read_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the block id latest read call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_block_id_latest_read(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_block_id_latest_read_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_block_id_latest_read(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!block_id_latest_read_callback)
    {
        status = block_id_latest_read_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_APP_BLOCK_ID_LATEST_READ, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Mock for the canonized transaction get call.
 *
 * \param req       The request payload.
 * \param size      The request payload size.
 *
 * \returns true if the request could be processed and false otherwise.
 */
bool mock_dataservice::mock_dataservice::
    mock_decode_and_dispatch_canonized_transaction_get(
        const void* request, size_t payload_size)
{
    bool retval = false;
    dataservice_request_canonized_transaction_get_t dreq;
    stringstream payout;
    string payload;
    uint32_t status = AGENTD_ERROR_DATASERVICE_NOT_FOUND;

    /* parse the request payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_canonized_transaction_get(
            request, payload_size, &dreq))
    {
        retval = false;
        goto done;
    }

    /* if the mock callback is set, call it. */
    if (!!canonized_transaction_get_callback)
    {
        status = canonized_transaction_get_callback(dreq, payout);
    }

    /* get the payload if set. */
    payload = payout.str();

    /* success. */
    retval = true;
    goto done;

done:
    mock_write_status(
        DATASERVICE_API_METHOD_APP_TRANSACTION_READ, 0U,
        status, payload.data(), payload.size());

    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 * \param artifact_id       The artifact_id for this request.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_payload_artifact_read(
        uint32_t child_index, const uint8_t* artifact_id)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_payload_artifact_read_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_APP_ARTIFACT_READ != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_payload_artifact_read(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index || 0 != memcmp(artifact_id, dreq.artifact_id, 16))
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 * \param block_height      The block height of the request.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_block_id_by_height_read(
        uint32_t child_index, uint64_t block_height)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_block_id_by_height_read_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_APP_BLOCK_ID_BY_HEIGHT_READ != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_block_id_by_height_read(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index || block_height != dreq.block_height)
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_block_id_latest_read(
        uint32_t child_index)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_block_id_latest_read_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_APP_BLOCK_ID_LATEST_READ != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_block_id_latest_read(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index)
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 * \param block_id          The block id for this request.
 * \param cert_size         The cert_size for this request.
 * \param cert              The cert for this request.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_block_make(
        uint32_t child_index, const uint8_t* block_id, size_t cert_size,
        const uint8_t* cert)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_block_make_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_APP_BLOCK_WRITE != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_block_make(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index || 0 != memcmp(block_id, dreq.block_id, 16) || cert_size != dreq.cert_size || 0 != memcmp(cert, dreq.cert, cert_size))
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 * \param block_id          The block id for this request.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_block_read(
        uint32_t child_index, const uint8_t* block_id)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_block_read_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_APP_BLOCK_READ != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_block_read(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index || 0 != memcmp(block_id, dreq.block_id, 16))
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 * \param txn_id            The transaction id for this request.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_canonized_transaction_get(
        uint32_t child_index, const uint8_t* txn_id)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_canonized_transaction_get_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_APP_TRANSACTION_READ != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_canonized_transaction_get(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index || 0 != memcmp(txn_id, dreq.txn_id, 16))
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_child_context_close(
        uint32_t child_index)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_child_context_close_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CLOSE != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_child_context_close(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index)
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param caps              The bitset for this child context.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_child_context_create(
        const void* caps)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_child_context_create_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_LL_CHILD_CONTEXT_CREATE != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_child_context_create(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        0 != memcmp(caps, dreq.caps, sizeof(dreq.caps)))
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 * \param key               The key for this global setting.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_global_setting_get(
        uint32_t child_index, uint64_t key)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_global_setting_get_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_READ != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_global_setting_get(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index || key != dreq.key)
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 * \param key               The key for this global setting.
 * \param val_size          The value size for this global setting.
 * \param val               The value.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_global_setting_set(
        uint32_t child_index, uint64_t key, size_t val_size,
        const void* gval)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_global_setting_set_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_APP_GLOBAL_SETTING_WRITE != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_global_setting_set(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index || key != dreq.key || val_size != dreq.val_size || 0 != memcmp(gval, dreq.val, val_size))
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 * \param txn_id            The transaction id for this request.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_transaction_drop(
        uint32_t child_index, const uint8_t* txn_id)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_transaction_drop_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_DROP != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_transaction_drop(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index || 0 != memcmp(txn_id, dreq.txn_id, 16))
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 * \param txn_id            The transaction id for this request.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_transaction_get(
        uint32_t child_index, const uint8_t* txn_id)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_transaction_get_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_READ != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_transaction_get(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index || 0 != memcmp(txn_id, dreq.txn_id, 16))
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_transaction_get_first(
        uint32_t child_index)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_transaction_get_first_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_FIRST_READ != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_transaction_get_first(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index)
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}

/**
 * \brief Return true if the next popped request matches this request.
 *
 * \param child_index       The child index for this request.
 * \param txn_id            The transaction id for this request.
 * \param artifact_id       The artifact id for this request.
 * \param cert_size         The certificate size for this request.
 * \param cert              The certificate for this request.
 */
bool mock_dataservice::mock_dataservice::
    request_matches_transaction_submit(
        uint32_t child_index, const uint8_t* txn_id,
        const uint8_t* artifact_id, size_t cert_size, const uint8_t* cert)
{
    bool retval = false;
    void* val = nullptr;
    uint32_t size = 0U;
    const uint8_t* breq = nullptr;
    uint32_t nmethod = 0U, method = 0U;
    dataservice_request_transaction_submit_t dreq;

    /* read a request from the test socket. */
    if (AGENTD_STATUS_SUCCESS != ipc_read_data_block(testsock, &val, &size))
    {
        retval = false;
        goto done;
    }

    /* make working with the request more convenient. */
    breq = (const uint8_t*)val;

    /* the payload should be at least large enough for the method. */
    if (size < sizeof(uint32_t))
    {
        retval = false;
        goto cleanup_val;
    }

    /* get the method. */
    memcpy(&nmethod, breq, sizeof(uint32_t));
    method = htonl(nmethod);

    /* increment breq past command. */
    breq += sizeof(uint32_t);

    /* decrement size. */
    size -= sizeof(uint32_t);

    /* verify the method. */
    if (DATASERVICE_API_METHOD_APP_PQ_TRANSACTION_SUBMIT != method)
    {
        retval = false;
        goto cleanup_val;
    }

    /* parse the requset payload. */
    if (AGENTD_STATUS_SUCCESS !=
        dataservice_decode_request_transaction_submit(
            breq, size, &dreq))
    {
        retval = false;
        goto cleanup_val;
    }

    /* verify the request. */
    if (
        child_index != dreq.hdr.child_index || 0 != memcmp(txn_id, dreq.txn_id, 16) || 0 != memcmp(artifact_id, dreq.artifact_id, 16) || cert_size != dreq.cert_size || 0 != memcmp(cert, dreq.cert, cert_size))
    {
        retval = false;
        goto cleanup_val;
    }

    /* successful match. */
    retval = true;
    goto cleanup_val;

cleanup_val:
    free(val);

done:
    return retval;
}
