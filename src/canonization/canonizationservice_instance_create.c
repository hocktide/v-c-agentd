/**
 * \file canonization/canonizationservice_instance_create.c
 *
 * \brief Create a canonization service instance.
 *
 * \copyright 2019-2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <unistd.h>
#include <string.h>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/parameters.h>

#include "canonizationservice_internal.h"

/* forward decls */
static void canonizationservice_instance_dispose(void* disposable);

/**
 * \brief Create the canonization service instance.
 *
 * \returns a properly created canonization service instance, or NULL on failure.
 */
canonizationservice_instance_t* canonizationservice_instance_create()
{
    int retval;

    /* allocate memory for the instance. */
    canonizationservice_instance_t* instance =
        (canonizationservice_instance_t*)
            malloc(sizeof(canonizationservice_instance_t));
    if (NULL == instance)
        return NULL;

    /* clear the instance. */
    memset(instance, 0, sizeof(canonizationservice_instance_t));

    /* set the dispose method. */
    instance->hdr.dispose = &canonizationservice_instance_dispose;

    /* initialize the malloc allocator. */
    malloc_allocator_options_init(&instance->alloc_opts);

    /* initialize the crypto suite for this instance. */
    retval =
        vccrypt_suite_options_init(
            &instance->crypto_suite, &instance->alloc_opts,
            VCCRYPT_SUITE_VELO_V1);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_allocator;
    }

    /* initialize the certificate builder options for this instance. */
    retval =
        vccert_builder_options_init(
            &instance->builder_opts, &instance->alloc_opts,
            &instance->crypto_suite);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_suite;
    }

    /* initialize linked list options for this instance. */
    retval =
        linked_list_options_init_ex(
            &instance->transaction_list_opts, &instance->alloc_opts, NULL, 0,
            &canonizationservice_transaction_list_element_dispose);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder_opts;
    }

    /* the transaction list only exists while it is being built. */
    instance->transaction_list = NULL;

    /* success. */
    return instance;

cleanup_builder_opts:
    dispose((disposable_t*)&instance->builder_opts);

cleanup_suite:
    dispose((disposable_t*)&instance->crypto_suite);

cleanup_allocator:
    dispose((disposable_t*)&instance->alloc_opts);

    /*cleanup_instance:*/
    memset(instance, 0, sizeof(canonizationservice_instance_t));
    free(instance);

    return NULL;
}

/**
 * \brief Dispose of a canonization service instance.
 *
 * \param disposable        The instance to dispose.
 */
static void canonizationservice_instance_dispose(void* disposable)
{
    canonizationservice_instance_t* instance =
        (canonizationservice_instance_t*)disposable;

    /* parameter sanity check. */
    MODEL_ASSERT(NULL != instance);

    /* clean up the linked list instance, if allocated. */
    if (NULL != instance->transaction_list)
    {
        dispose((disposable_t*)instance->transaction_list);
        /* TODO - linked_list_t's dispose method should handle this. */
        memset(instance->transaction_list, 0, sizeof(linked_list_t));
        free(instance->transaction_list);
        instance->transaction_list = NULL;
    }

    /* clean up the linked list options. */
    dispose((disposable_t*)&instance->transaction_list_opts);

    /* clean up the builder options. */
    dispose((disposable_t*)&instance->builder_opts);

    /* clean up the crypto suite. */
    dispose((disposable_t*)&instance->crypto_suite);

    /* clean up the allocator options. */
    dispose((disposable_t*)&instance->alloc_opts);

    /* clear the data structure. */
    memset(instance, 0, sizeof(canonizationservice_instance_t));
}
