/**
 * \file mocks/dataservice.h
 *
 * Private header for the dataservice mock.
 *
 * \copyright 2019 Velo-Payments, Inc.  All rights reserved.
 */

#ifndef TEST_MOCK_MOCK_DATASERVICE_HEADER_GUARD
#define TEST_MOCK_MOCK_DATASERVICE_HEADER_GUARD

/* this header will only work for C++. */
#if !defined(__cplusplus)
#error This is a C++ header file.
#endif /*! defined(__cplusplus)*/

#include <functional>
#include <list>
#include <memory>
#include <sys/types.h>

#include "../../src/dataservice/dataservice_protocol_internal.h"

namespace mock_dataservice {
/**
     * \brief Mock request.
     */
struct mock_request
{
    size_t size;
    void* data;
};

/**
     * \brief Mock request deleter.
     */
struct mock_request_deleter
{
    void operator()(mock_request* p) const
    {
        memset(p->data, 0, p->size);
        free(p->data);
        memset(p, 0, sizeof(mock_request));
        free(p);
    }
};

/**
     * \brief Mock dataservice.
     *
     * This class is used to mock the dataservice for isolation tests.
     */
class mock_dataservice {
public:
    /**
         * \brief Create a mock dataservice instance that will listen on the
         * given socket when started.
         *
         * \param _datasock         The socket used to listen for dataservice
         *                          requests.
         */
    mock_dataservice(int _datasock);

    /**
         * \brief Make sure to stop the mock dataservice if running on
         * destruction.
         */
    ~mock_dataservice();

    /**
         * \brief Start the mock dataservice with the current mock settings.
         */
    void start();

    /**
         * \brief Stop the mock dataservice if running.
         */
    void stop();

    /**
         * \brief Register a mock callback for payload_artifact_read.
         *
         * \param cb                The callback to register.
         */
    void register_callback_payload_artifact_read(
        std::function<
            int(const dataservice_request_payload_artifact_read_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for block_id_by_height_read.
         *
         * \param cb                The callback to register.
         */
    void register_callback_block_id_by_height_read(
        std::function<
            int(const dataservice_request_block_id_by_height_read_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for block_id_latest_read.
         *
         * \param cb                The callback to register.
         */
    void register_callback_block_id_latest_read(
        std::function<
            int(const dataservice_request_block_id_latest_read_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for block_make.
         *
         * \param cb                The callback to register.
         */
    void register_callback_block_make(
        std::function<
            int(const dataservice_request_block_make_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for block_read.
         *
         * \param cb                The callback to register.
         */
    void register_callback_block_read(
        std::function<
            int(const dataservice_request_block_read_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for canonized_transaction_get.
         *
         * \param cb                The callback to register.
         */
    void register_callback_canonized_transaction_get(
        std::function<
            int(const dataservice_request_canonized_transaction_get_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for child_context_close.
         *
         * \param cb                The callback to register.
         */
    void register_callback_child_context_close(
        std::function<
            int(const dataservice_request_child_context_close_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for child_context_create.
         *
         * \param cb                The callback to register.
         */
    void register_callback_child_context_create(
        std::function<
            int(const dataservice_request_child_context_create_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for global_setting_get.
         *
         * \param cb                The callback to register.
         */
    void register_callback_global_setting_get(
        std::function<
            int(const dataservice_request_global_setting_get_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for global_setting_set.
         *
         * \param cb                The callback to register.
         */
    void register_callback_global_setting_set(
        std::function<
            int(const dataservice_request_global_setting_set_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for transaction_drop.
         *
         * \param cb                The callback to register.
         */
    void register_callback_transaction_drop(
        std::function<
            int(const dataservice_request_transaction_drop_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for transaction_get.
         *
         * \param cb                The callback to register.
         */
    void register_callback_transaction_get(
        std::function<
            int(const dataservice_request_transaction_get_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for transaction_get_first.
         *
         * \param cb                The callback to register.
         */
    void register_callback_transaction_get_first(
        std::function<
            int(const dataservice_request_transaction_get_first_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Register a mock callback for transaction_submit.
         *
         * \param cb                The callback to register.
         */
    void register_callback_transaction_submit(
        std::function<
            int(const dataservice_request_transaction_submit_t&,
                std::ostream&)>
            cb);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         * \param artifact_id       The artifact_id for this request.
         */
    bool request_matches_payload_artifact_read(
        uint32_t child_index, const uint8_t* artifact_id);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         * \param block_height      The block height of the request.
         */
    bool request_matches_block_id_by_height_read(
        uint32_t child_index, uint64_t block_height);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         */
    bool request_matches_block_id_latest_read(
        uint32_t child_index);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         * \param block_id          The block id for this request.
         * \param cert_size         The cert_size for this request.
         * \param cert              The cert for this request.
         */
    bool request_matches_block_make(
        uint32_t child_index, const uint8_t* block_id, size_t cert_size,
        const uint8_t* cert);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         * \param block_id          The block id for this request.
         */
    bool request_matches_block_read(
        uint32_t child_index, const uint8_t* block_id);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         * \param txn_id            The transaction id for this request.
         */
    bool request_matches_canonized_transaction_get(
        uint32_t child_index, const uint8_t* txn_id);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         */
    bool request_matches_child_context_close(
        uint32_t child_index);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param caps              The bitset for this child context.
         */
    bool request_matches_child_context_create(
        const void* caps);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         * \param key               The key for this global setting.
         */
    bool request_matches_global_setting_get(
        uint32_t child_index, uint64_t key);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         * \param key               The key for this global setting.
         * \param val_size          The value size for this global setting.
         * \param val               The value.
         */
    bool request_matches_global_setting_set(
        uint32_t child_index, uint64_t key, size_t val_size,
        const void* val);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         * \param txn_id            The transaction id for this request.
         */
    bool request_matches_transaction_drop(
        uint32_t child_index, const uint8_t* txn_id);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         * \param txn_id            The transaction id for this request.
         */
    bool request_matches_transaction_get(
        uint32_t child_index, const uint8_t* txn_id);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         */
    bool request_matches_transaction_get_first(
        uint32_t child_index);

    /**
         * \brief Return true if the next popped request matches this request.
         *
         * \param child_index       The child index for this request.
         * \param txn_id            The transaction id for this request.
         * \param artifact_id       The artifact id for this request.
         * \param cert_size         The certificate size for this request.
         * \param cert              The certificate for this request.
         */
    bool request_matches_transaction_submit(
        uint32_t child_index, const uint8_t* txn_id,
        const uint8_t* artifact_id, size_t cert_size, const uint8_t* cert);

private:
    int datasock;
    bool running;
    int testsock;
    int mocksock;
    pid_t mock_pid;
    std::list<std::shared_ptr<mock_request>> request_list;

    /* mock callbacks. */
    std::function<
        int(const dataservice_request_payload_artifact_read_t&,
            std::ostream&)>
        payload_artifact_read_callback;
    std::function<
        int(const dataservice_request_block_id_by_height_read_t&,
            std::ostream&)>
        block_id_by_height_read_callback;
    std::function<
        int(const dataservice_request_block_id_latest_read_t&,
            std::ostream&)>
        block_id_latest_read_callback;
    std::function<
        int(const dataservice_request_block_make_t&,
            std::ostream&)>
        block_make_callback;
    std::function<
        int(const dataservice_request_block_read_t&,
            std::ostream&)>
        block_read_callback;
    std::function<
        int(const dataservice_request_canonized_transaction_get_t&,
            std::ostream&)>
        canonized_transaction_get_callback;
    std::function<
        int(const dataservice_request_child_context_close_t&,
            std::ostream&)>
        child_context_close_callback;
    std::function<
        int(const dataservice_request_child_context_create_t&,
            std::ostream&)>
        child_context_create_callback;
    std::function<
        int(const dataservice_request_global_setting_get_t&,
            std::ostream&)>
        global_setting_get_callback;
    std::function<
        int(const dataservice_request_global_setting_set_t&,
            std::ostream&)>
        global_setting_set_callback;
    std::function<
        int(const dataservice_request_transaction_drop_t&,
            std::ostream&)>
        transaction_drop_callback;
    std::function<
        int(const dataservice_request_transaction_get_t&,
            std::ostream&)>
        transaction_get_callback;
    std::function<
        int(const dataservice_request_transaction_get_first_t&,
            std::ostream&)>
        transaction_get_first_callback;
    std::function<
        int(const dataservice_request_transaction_submit_t&,
            std::ostream&)>
        transaction_submit_callback;

    /**
         * \brief Run the mock dataservice process.
         *
         * Read request packets from the process and write canned response
         * packets, possibly using the mock override callbacks.
         */
    void mock_process();

    /**
         * \brief Read and dispatch one request.
         *
         * \returns true if a request was read, and false if anything goes
         *          wrong (e.g. a socket was closed).
         */
    bool mock_read_and_dispatch();

    /**
         * \brief Write the status back to the caller.
         *
         * \param method    The method requested.
         * \param offset    The child offset.
         * \param status    The status code.
         * \param payload   The response payload, or ptr if none.
         * \param size      The response payload size.
         */
    void mock_write_status(
        uint32_t method, uint32_t offset, uint32_t status,
        const void* payload, size_t size);

    /**
         * \brief Mock for the root context create call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_root_context_create(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the root capabilities call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_root_context_reduce_caps(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the child context create call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_child_context_create(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the child context close call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_child_context_close(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the global setting get call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_global_setting_get(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the global setting set call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_global_setting_set(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the transaction submit call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_transaction_submit(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the transaction get first call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_transaction_get_first(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the transaction get call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_transaction_get(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the transaction drop call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_transaction_drop(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the artifact read call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_artifact_read(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the block make call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_block_make(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the block read call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_block_read(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the block id by height read call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_block_id_by_height_read(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the block id latest read call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_block_id_latest_read(
        const void* request, size_t payload_size);

    /**
         * \brief Mock for the canonized transaction get call.
         *
         * \param req       The request payload.
         * \param size      The request payload size.
         *
         * \returns true if the request could be processed and false otherwise.
         */
    bool mock_decode_and_dispatch_canonized_transaction_get(
        const void* request, size_t payload_size);
};
}

#endif /*TEST_MOCK_MOCK_DATASERVICE_HEADER_GUARD*/
