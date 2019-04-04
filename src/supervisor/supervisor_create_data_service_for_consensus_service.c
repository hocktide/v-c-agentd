/**
 * \file supervisor/supervisor_create_data_service_for_consensus_service.c
 *
 * \brief Create the data service for the consensus service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/control.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>

#include "supervisor_private.h"

/**
 * \brief Create a data service instance for the consensus service as a
 * process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the data service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              data service.  This configuration must be
 *                              valid for the lifetime of the service.
 * \param data_socket           Pointer to the descriptor to receive the data
 *                              socket.
 * \param log_socket            Pointer to the descriptor holding the log socket
 *                              for this instance.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_data_service_for_consensus_service(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, int* data_socket, int* log_socket)
{
    int retval;

    /* allocate memory for the dataservice process. */
    dataservice_process_t* data_proc =
        (dataservice_process_t*)malloc(sizeof(dataservice_process_t));
    if (NULL == data_proc)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set up data_proc structure. */
    memset(data_proc, 0, sizeof(dataservice_process_t));
    data_proc->hdr.hdr.dispose = &supervisor_dispose_data_service;
    data_proc->hdr.init_method = &supervisor_start_data_service;
    data_proc->bconf = bconf;
    data_proc->conf = conf;
    data_proc->log_socket = log_socket;

    /* save the supervisor data socket to be set later. */
    data_proc->supervisor_data_socket = data_socket;

    /* set the reduced capabilities for this instance. */
    BITCAP_INIT_FALSE(data_proc->reducedcaps);
    /* allow for the creation of child contexts. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CREATE);
    /* allow for the closing of child contexts. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_LL_CHILD_CONTEXT_CLOSE);
    /* consensus service can read latest block ID. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_LATEST_READ);
    /* consensus service can read next block ID. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_NEXT_READ);
    /* consensus service can read previous block ID. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_PREV_READ);
    /* consensus service can read the block ID for a transaction. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_WITH_TRANSACTION_READ);
    /* consensus service can read a block. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_READ);
    /* consensus service can read a transaction by ID. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_TRANSACTION_READ);
    /* consensus service can submit a transaction to the process queue. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_SUBMIT);
    /* consensus service can read the first txn from process queue. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_FIRST_READ);
    /* consensus service can read a transaction from the process queue. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_READ);
    /* consensus service can drop a transaction from the process queue. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_PQ_TRANSACTION_DROP);
    /* consensus service can read an artifact by ID. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_ARTIFACT_READ);
    /* consensus service can write a block to the block table. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_WRITE);
    /* consensus service can query a block ID by block height. */
    BITCAP_SET_TRUE(data_proc->reducedcaps,
        DATASERVICE_API_CAP_APP_BLOCK_ID_BY_HEIGHT_READ);

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    *svc = (process_t*)data_proc;
    goto done;

done:
    return retval;
}
