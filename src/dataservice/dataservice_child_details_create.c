/**
 * \file dataservice/dataservice_child_details_create.c
 *
 * \brief Create a child details structure.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/* forward decls */
static void dataservice_child_context_dispose(void* disposable);

/**
 * \brief Create a child details structure for the given dataservice instance.
 *
 * \param inst          The instance in which this child context is created.
 * \param offset        Pointer to the offset that is updated with this child
 *                      context offset in the children array.
 *
 * \returns 0 on success, and non-zero on failure.
 */
int dataservice_child_details_create(dataservice_instance_t* inst, int* offset)
{
    int retval = 0;
    dataservice_child_details_t* child = NULL;

    /* if there is not an instance available, this operation fails. */
    if (NULL == inst->child_head)
    {
        retval = 1;
        goto done;
    }

    /* get a child instance and the offset. */
    if (NULL == inst->child_head)
    {
        /* all children are allocated. */
        retval = 2;
        goto done;
    }

    /* complete the allocation of the child. */
    child = inst->child_head;
    inst->child_head = child->next;
    *offset = child - &inst->children[0];

    /* clear the child instance prior to initialization. */
    memset(child, 0, sizeof(dataservice_child_details_t));

    /* set the dispose method. */
    child->hdr.dispose = &dataservice_child_context_dispose;

    /* success. */
    retval = 0;
    goto done;

done:
    return retval;
}

/**
 * Dispose a child details structure.
 *
 * \param disposable        The child details structure to dispose.
 */
static void dataservice_child_context_dispose(void* disposable)
{
    dataservice_child_details_t* child =
        (dataservice_child_details_t*)disposable;

    /* clear the structure. */
    memset(child, 0, sizeof(dataservice_child_details_t));
}
