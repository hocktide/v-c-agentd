/**
 * \file randomservice/randomservice_instance_create.c
 *
 * \brief Create a randomservice instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/randomservice/private/randomservice.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <vpr/parameters.h>

#include "randomservice_internal.h"

/* forward decls */
static void randomservice_instance_dispose(void* disposable);

/**
 * \brief Create a randomservice instance.
 *
 * \param random        File descriptor pointing to /dev/random.
 *
 * \returns a properly created randomservice instance, or NULL on failure.
 */
randomservice_root_context_t* randomservice_instance_create(int random)
{
    /* parameter sanity check. */
    MODEL_ASSERT(random >= 0);

    /* allocate memory for the instance. */
    randomservice_root_context_t* instance = (randomservice_root_context_t*)
        malloc(sizeof(randomservice_root_context_t));
    if (NULL == instance)
    {
        return NULL;
    }

    /* clear the instance. */
    memset(instance, 0, sizeof(randomservice_root_context_t));

    /* set the dispose method. */
    instance->hdr.dispose = &randomservice_instance_dispose;

    /* set the random handle. */
    instance->random_fd = random;

    /* success. */
    return instance;
}

/**
 * \brief Dispose of a randomservice instance.
 *
 * \param disposable        The instance to dispose.
 */
static void randomservice_instance_dispose(void* disposable)
{
    randomservice_root_context_t* instance =
        (randomservice_root_context_t*)disposable;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != instance);

    /* clear the structure. */
    memset(instance, 0, sizeof(randomservice_root_context_t));
}
