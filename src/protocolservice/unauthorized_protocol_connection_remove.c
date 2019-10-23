/**
 * \file protocolservice/unauthorized_protocol_connection_remove.c
 *
 * \brief Remove a connection from its current list.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/**
 * \brief Remove a protocol connection from its current list.
 *
 * \param head          Pointer to the head of the list.
 * \param conn          The connection to remove.
 */
void unauthorized_protocol_connection_remove(
    unauthorized_protocol_connection_t** head,
    unauthorized_protocol_connection_t* conn)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != head);
    MODEL_ASSERT(NULL != conn);

    /* if prev is set, fix it up. */
    if (NULL != conn->prev)
    {
        conn->prev->next = conn->next;
    }
    else
    {
        /* if prev is null, then this must be head. */
        MODEL_ASSERT(conn == *head);
        *head = conn->next;
    }

    /* if next is set, fix it up. */
    if (NULL != conn->next)
    {
        conn->next->prev = conn->prev;
    }

    /* this node is now orphaned.  Update pointers to reflect this. */
    conn->prev = conn->next = NULL;
}
