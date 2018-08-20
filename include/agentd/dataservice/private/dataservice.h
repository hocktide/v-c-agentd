/**
 * \file agentd/dataservice/private/dataservice.h
 *
 * \brief Private internal API for the data service.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_DATASERVICE_PRIVATE_DATASERVICE_HEADER_GUARD
#define AGENTD_DATASERVICE_PRIVATE_DATASERVICE_HEADER_GUARD

#include <agentd/bitcap.h>
#include <agentd/dataservice.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Context structure for the internal data service.
 */
typedef struct dataservice_private_context
{
    /**
     * \brief This structure is disposable.
     */
    disposable_t hdr;

    /**
     * \brief Root capabilities bitset for this context.
     */
    BITCAP(apicaps, DATASERVICE_API_CAP_BITS_MAX);

    /**
     * \brief Opaque pointer to the database details structure.
     */
    void* details;

} dataservice_root_context_t;

/**
 * \brief Child context structure for the data service.
 *
 * This structure is used to further reduce capabilities in the root context for
 * a set of operations.
 */
typedef struct dataservice_child_context
{
    /**
     * \brief the root context for this interface context.
     */
    dataservice_root_context_t* root;

    /**
     * \brief Child context capabilities.
     */
    BITCAP(childcaps, DATASERVICE_API_CAP_BITS_MAX);

} dataservice_child_context_t;

/**
 * \brief Create a root data service context.
 *
 * \param ctx           The private data service context to initialize.
 * \param datadir       The data directory for this private data service.
 *
 * \returns 0 on success and non-zero on failure.
 */
int dataservice_root_context_init(
    dataservice_root_context_t* ctx, const char* datadir);

/**
 * \brief Reduce the root capabilities of a private data service instance.
 *
 * \param ctx           The private data service context to modify.
 * \param caps          The capabilities bitset to use for the reduction
 *                      operation.  It is ANDed against the current capabilities
 *                      in the context to create a reduced context.  The data
 *                      structure must be the same size as the capabilities
 *                      structure defined in dataservice_root_context_t.
 *
 * \returns 0 on success and non-zero on failure.
 */
int dataservice_root_context_reduce_capabilities(
    dataservice_root_context_t* ctx, uint32_t* caps);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_DATASERVICE_PRIVATE_DATASERVICE_HEADER_GUARD*/
