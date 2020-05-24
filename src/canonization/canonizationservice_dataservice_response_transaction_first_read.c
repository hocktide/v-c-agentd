/**
 * \file
 * canonization/canonizationservice_dataservice_response_transaction_first_read.c
 *
 * \brief Handle the response from the data service transaction first read call.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/dataservice/api.h>
#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Handle the response from the data service transaction first read.
 *
 * \param instance      The canonization service instance.
 * \param resp          The response from the data service.
 * \param resp_size     The size of the response from the data service.
 */
void canonizationservice_dataservice_response_transaction_first_read(
    canonizationservice_instance_t* instance, const uint32_t* resp,
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
        canonizationservice_exit_event_loop(instance);
        goto done;
    }
    else if (AGENTD_STATUS_SUCCESS == retval && AGENTD_ERROR_DATASERVICE_NOT_FOUND == dresp.hdr.status)
    {
        canonizationservice_child_context_close(instance);
        goto done;
    }

    /* if this transaction has not yet been attested, then we are done. */
    if (DATASERVICE_TRANSACTION_NODE_STATE_ATTESTED !=
        ntohl(dresp.node.net_txn_state))
    {
        canonizationservice_child_context_close(instance);
        goto done;
    }

    /* create a transaction instance to hold this txn. */
    canonizationservice_transaction_t* txn =
        (canonizationservice_transaction_t*)malloc(
            sizeof(canonizationservice_transaction_t) + dresp.data_size);
    if (NULL == txn)
    {
        canonizationservice_exit_event_loop(instance);
        goto done;
    }

    /* set the disposer. */
    txn->hdr.dispose = &canonizationservice_transaction_dispose;

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

        canonizationservice_exit_event_loop(instance);
        goto done;
    }

    /* if we've reached our max count, we're done. */
    if (instance->transaction_list->elements == instance->block_max_transactions)
    {
        canonizationservice_block_make(instance);
        goto done;
    }

    /* if the next node is the end node, we're done. */
    if (dataservice_api_node_ref_is_end(txn->node.next))
    {
        canonizationservice_block_make(instance);
        goto done;
    }

    /* send the request to read the first transaction from the transaction
     * process queue. */
    retval =
        dataservice_api_sendreq_transaction_get(
            instance->data, instance->data_child_context, txn->node.next);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        canonizationservice_exit_event_loop(instance);
        goto done;
    }

    /* set the write callback for the dataservice socket. */
    ipc_set_writecb_noblock(
        instance->data, &canonizationservice_data_write,
        instance->loop_context);

    /* success. */
    goto done;

done:;
}
