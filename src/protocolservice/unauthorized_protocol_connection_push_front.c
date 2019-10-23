/**
 * \file protocolservice/unauthorized_protocol_connection_push_front.c
 *
 * \brief Add a connection to the head of a list.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Push a protocol connection onto the given list.
 *
 * \param head          Pointer to the head of the list.
 * \param conn          The connection to push.
 */
void unauthorized_protocol_connection_push_front(
    unauthorized_protocol_connection_t** head,
    unauthorized_protocol_connection_t* conn)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != head);
    MODEL_ASSERT(NULL != conn);
    MODEL_ASSERT(NULL == conn->prev);
    MODEL_ASSERT(NULL == conn->next);

    /* if head is set, then fix up head->prev. */
    if (NULL != *head)
    {
        MODEL_ASSERT(NULL == (*head)->prev);
        (*head)->prev = conn;
    }

    /* Fix up conn. */
    conn->next = *head;
    conn->prev = NULL;

    /* update head. */
    *head = conn;
}
