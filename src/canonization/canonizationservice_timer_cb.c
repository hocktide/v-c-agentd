/**
 * \file canonization/canonizationservice_timer_cb.c
 *
 * \brief Timer callback for the canonization service.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/canonizationservice.h>
#include <agentd/canonizationservice/api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/**
 * \brief Timer callback for the canonization service.
 *
 * This callback is called periodically to check the process queue for attested
 * certificates.  When these are found, these are used to build the next block
 * that is appended to the blockchain.
 *
 * \param timer         The timer context for this call.
 * \param context       The user context for this call, which is expected to be
 *                      a \ref canonizationservice_instance_t instance.
 */
void canonizationservice_timer_cb(
    ipc_timer_context_t* UNUSED(timer), void* context)
{
    int retval;

    canonizationservice_instance_t* instance =
        (canonizationservice_instance_t*)context;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != instance);
    MODEL_ASSERT(NULL != timer);

    /* allocate memory for the transaction list. */
    MODEL_ASSERT(NULL == instance->transaction_list);
    instance->transaction_list = (linked_list_t*)malloc(sizeof(linked_list_t));
    if (NULL == instance->transaction_list)
    {
        ipc_exit_loop(instance->loop_context);
        goto done;
    }

    /* initialize the transaction list. */
    retval =
        linked_list_init(
            &instance->transaction_list_opts, instance->transaction_list);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        goto free_transaction_list;
    }

    /* send a request to the random service. */
    retval = canonizationservice_write_block_id_request(instance);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        ipc_exit_loop(instance->loop_context);
        goto dispose_transaction_list;
    }

    /* success. */
    goto done;

dispose_transaction_list:
    dispose((disposable_t*)instance->transaction_list);

free_transaction_list:
    free(instance->transaction_list);
    instance->transaction_list = NULL;

done:;
}
