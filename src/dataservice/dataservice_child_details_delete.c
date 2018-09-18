/**
 * \file dataservice/dataservice_child_details_delete.c
 *
 * \brief Reclaim a child details structure.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/private/dataservice.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "dataservice_internal.h"

/**
 * \brief Reclaim a child details structure.
 *
 * \param inst          The instance to which this structure belongs.
 * \param offset        The offset to reclaim.
 */
void dataservice_child_details_delete(dataservice_instance_t* inst, int offset)
{
    dataservice_child_details_t* child = &inst->children[offset];

    /* dispose of the child. */
    dispose((disposable_t*)child);

    /* place the child on the free store. */
    child->next = inst->child_head;
    inst->child_head = child->next;
}
