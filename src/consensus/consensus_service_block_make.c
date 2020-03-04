/**
 * \file consensus/consensus_service_block_make.c
 *
 * \brief Build a new block for the blockchain, using the currently attested
 * transactions.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/consensusservice.h>
#include <agentd/consensusservice/api.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <time.h>
#include <vccert/certificate_types.h>
#include <vccert/fields.h>
#include <vpr/parameters.h>

#include "consensusservice_internal.h"

/**
 * \brief Build a new block for the blockchain, using the currently attested
 * transactions.
 *
 * \param instance      The consensus service instance.
 */
int consensus_service_block_make(
    consensusservice_instance_t* instance)
{
    int retval = 0;
    size_t block_size = 0;
    vccert_builder_context_t builder;

    /* Do we have transactions to put in a block? */
    if (0 == instance->transaction_list->elements)
    {
        consensus_service_child_context_close(instance);
        retval = AGENTD_STATUS_SUCCESS;
        goto done;
    }

    /* Things we need before this point: */
    /*   * UUID for new block - random service query. */
    /*   * UUID for previous block - data service query. */
    /*   * Signature for previous block - data service query. */

    /* compute block certificate header size. */
    block_size =
        /* certificate version. */
        FIELD_TYPE_SIZE + FIELD_SIZE_SIZE + sizeof(uint32_t)
        /* transaction timestamp */
        + FIELD_TYPE_SIZE + FIELD_SIZE_SIZE + sizeof(uint64_t)
        /* crypto suite. */
        + FIELD_TYPE_SIZE + FIELD_SIZE_SIZE + sizeof(uint16_t)
        /* certificate type. */
        + FIELD_TYPE_SIZE + FIELD_SIZE_SIZE + 16
        /* block id. */
        + FIELD_TYPE_SIZE + FIELD_SIZE_SIZE + 16
        /* previous block id. */
        + FIELD_TYPE_SIZE + FIELD_SIZE_SIZE + 16
        /* previous block hash. */
        + FIELD_TYPE_SIZE + FIELD_SIZE_SIZE +
        instance->crypto_suite.hash_opts.hash_size
        /* block height. */
        + FIELD_TYPE_SIZE + FIELD_SIZE_SIZE + sizeof(uint64_t)
        /* signer id. */
        + FIELD_TYPE_SIZE + FIELD_SIZE_SIZE + 16
        /* signature. */
        + FIELD_TYPE_SIZE + FIELD_SIZE_SIZE +
        instance->crypto_suite.sign_opts.signature_size;

    /* iterate through each transaction in our transaction list. */
    linked_list_element_t* elem = instance->transaction_list->first;
    while (NULL != elem)
    {
        /* get the transaction data for this element. */
        consensusservice_transaction_t* txn =
            (consensusservice_transaction_t*)elem->data;

        /* increase the block size by the field size for this transaction. */
        block_size += FIELD_TYPE_SIZE + FIELD_SIZE_SIZE + txn->cert_size;

        /* go to the next list element. */
        elem = elem->next;
    }

    /* 2.  Create builder instance. */
    retval = vccert_builder_init(&instance->builder_opts, &builder, block_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        goto done;
    }

    /* TODO - add previous block uuid. */
    /* TODO - add previous block signature. */

    /* add certificate version. */
    uint32_t cert_version = 0x00010000;
    retval =
        vccert_builder_add_short_uint32(
            &builder, VCCERT_FIELD_TYPE_CERTIFICATE_VERSION, cert_version);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        goto cleanup_builder;
    }

    /* get current time. */
    time_t timestamp = time(NULL);
    if (timestamp < 0)
    {
        ipc_exit_loop(instance->loop_context);
        goto cleanup_builder;
    }

    /* add time to the builder. */
    uint64_t timestamp64 = timestamp;
    retval =
        vccert_builder_add_short_uint64(
            &builder, VCCERT_FIELD_TYPE_CERTIFICATE_VALID_FROM, timestamp64);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        goto cleanup_builder;
    }

    /* add crypto suite to the builder. */
    uint16_t crypto_suite = 0x0001;
    retval =
        vccert_builder_add_short_uint16(
            &builder, VCCERT_FIELD_TYPE_CERTIFICATE_CRYPTO_SUITE, crypto_suite);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        goto cleanup_builder;
    }

    /* add certificate type to builder. */
    retval =
        vccert_builder_add_short_buffer(
            &builder, VCCERT_FIELD_TYPE_CERTIFICATE_TYPE,
            vccert_certificate_type_uuid_txn_block,
            sizeof(vccert_certificate_type_uuid_txn_block));
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        goto cleanup_builder;
    }

    /* add block id to the builder. */
    retval =
        vccert_builder_add_short_buffer(
            &builder, VCCERT_FIELD_TYPE_BLOCK_UUID,
            instance->block_id, sizeof(instance->block_id));
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        goto cleanup_builder;
    }

    /* TODO - add previous block id to the builder. */
    /* TODO - add previous block hash to the builder. */
    /* TODO - add block height to the builder. */

    /* add each transaction to the certificate. */
    elem = instance->transaction_list->first;
    while (NULL != elem)
    {
        /* get the transaction data for this element. */
        consensusservice_transaction_t* txn =
            (consensusservice_transaction_t*)elem->data;

        /* add the transaction certificate to the block. */
        retval =
            vccert_builder_add_short_buffer(
                &builder, VCCERT_FIELD_TYPE_WRAPPED_TRANSACTION_TUPLE,
                txn->cert, txn->cert_size);
        if (AGENTD_STATUS_SUCCESS != retval)
        {
            ipc_exit_loop(instance->loop_context);
            goto cleanup_builder;
        }

        /* go to the next list element. */
        elem = elem->next;
    }

    /* TODO - sign certificate. */

    /* TODO - hack to get block bytes.  Replace with sign above. */
    size_t block_cert_size;
    const uint8_t* block_cert_bytes =
        vccert_builder_emit(&builder, &block_cert_size);
    if (NULL == block_cert_bytes)
    {
        ipc_exit_loop(instance->loop_context);
        goto cleanup_builder;
    }

    /* call dataservice_sendreq_block_make. */
    retval =
        dataservice_api_sendreq_block_make(
            instance->data, instance->data_child_context, instance->block_id,
            block_cert_bytes, block_cert_size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        goto cleanup_builder;
    }

    /* update our state. */
    instance->state = CONSENSUS_SERVICE_STATE_WAITRESP_BLOCK_MAKE;

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        instance->data, &consensus_service_data_write, instance->loop_context);

    /* success. */

cleanup_builder:
    dispose((disposable_t*)&builder);

done:
    return retval;
}
