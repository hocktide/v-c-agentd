/**
 * \file consensus/consensusservice_instance_create.c
 *
 * \brief Create a consensus service instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <cbmc/model_assert.h>
#include <unistd.h>
#include <string.h>
#include <vpr/parameters.h>

#include "consensusservice_internal.h"

/* forward decls */
static void consensusservice_instance_dispose(void* disposable);

/**
 * \brief Create the consensus service instance.
 *
 * \returns a properly created consensus service instance, or NULL on failure.
 */
consensusservice_instance_t* consensusservice_instance_create()
{
    /* allocate memory for the instance. */
    consensusservice_instance_t* instance =
        (consensusservice_instance_t*)
            malloc(sizeof(consensusservice_instance_t));
    if (NULL == instance)
        return NULL;

    /* clear the instance. */
    memset(instance, 0, sizeof(consensusservice_instance_t));

    /* set the dispose method. */
    instance->hdr.dispose = &consensusservice_instance_dispose;

    /* success. */
    return instance;
}

/**
 * \brief Dispose of a consensus service instance.
 *
 * \param disposable        The instance to dispose.
 */
static void consensusservice_instance_dispose(void* disposable)
{
    consensusservice_instance_t* instance =
        (consensusservice_instance_t*)disposable;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != instance);

    /* clear the data structure. */
    memset(instance, 0, sizeof(consensusservice_instance_t));
}
