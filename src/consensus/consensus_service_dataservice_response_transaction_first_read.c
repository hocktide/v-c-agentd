/**
 * \file consensus/consensus_service_dataservice_response_transaction_first_read.c
 *
 * \brief Handle the response from the data service transaction first read call.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/consensusservice.h>
#include <agentd/consensusservice/api.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "consensusservice_internal.h"

/**
 * \brief Handle the response from the data service transaction first read.
 *
 * \param instance      The consensus service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void consensus_service_dataservice_response_transaction_first_read(
    consensusservice_instance_t* instance, const uint32_t* resp,
    const size_t resp_size)
{
    int retval;
    dataservice_response_transaction_get_first_t dresp;

    /* decode the response. */
    retval =
        dataservice_decode_response_transaction_get_first(
            resp, resp_size, &dresp);
    if (AGENTD_STATUS_SUCCESS != retval || (AGENTD_STATUS_SUCCESS != dresp.hdr.status && AGENTD_ERROR_DATASERVICE_NOT_FOUND != dresp.hdr.status))
    {
        ipc_exit_loop(instance->loop_context);
        goto done;
    }
    else if (AGENTD_STATUS_SUCCESS == retval && AGENTD_ERROR_DATASERVICE_NOT_FOUND == dresp.hdr.status)
    {
        consensus_service_child_context_close(instance);
        goto done;
    }

    /* if this transaction has not yet been attested, then we are done. */
    if (DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED !=
        ntohl(dresp.node.net_txn_state))
    {
        consensus_service_child_context_close(instance);
        goto done;
    }

    /* create a transaction instance to hold this txn. */
    consensusservice_transaction_t* txn =
        (consensusservice_transaction_t*)malloc(
            sizeof(consensusservice_transaction_t) + dresp.data_size);
    if (NULL == txn)
    {
        ipc_exit_loop(instance->loop_context);
        goto done;
    }

    /* set the disposer. */
    txn->hdr.dispose = &consensusservice_transaction_dispose;

    /* copy the node data. */
    memcpy(&txn->node, &dresp.node, sizeof(txn->node));

    /* copy the certificate. */
    txn->cert_size = dresp.data_size;
    memcpy(&txn->cert, dresp.data, txn->cert_size);

    /* insert this transaction into the transaction list. */
    retval = linked_list_insert_end(instance->transaction_list, txn);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        dispose((disposable_t*)txn);
        free(txn);

        ipc_exit_loop(instance->loop_context);
        goto done;
    }

    /* if we've reached our max count, we're done. */
    if (instance->transaction_list->elements == instance->block_max_transactions)
    {
        consensus_service_block_make(instance);
        goto done;
    }

    /* if the next node is the end node, we're done. */
    if (dataservice_api_node_ref_is_end(txn->node.next))
    {
        consensus_service_block_make(instance);
        goto done;
    }

    /* send the request to read the first transaction from the transaction
     * process queue. */
    retval =
        dataservice_api_sendreq_transaction_get(
            instance->data, instance->data_child_context, txn->node.next);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        goto done;
    }

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        instance->data, &consensus_service_data_write, instance->loop_context);

    /* success. */
    goto done;

done:;
}
