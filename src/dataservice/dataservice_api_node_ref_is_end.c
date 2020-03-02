/**
 * \file dataservice/dataservice_api_node_ref_is_end.c
 *
 * \brief Determine whether a node id reference is the end node id reference.
 *
 * \copyright 2020 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/dataservice/api.h>
#include <cbmc/model_assert.h>
#include <vccrypt/compare.h>
#include <vpr/parameters.h>

#ifdef CBMC
#include <sys/contiguous_space.h>
#endif

static const uint8_t dataservice_api_node_ref_end[16] = {
    0xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff,
    0xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff, 0Xff
};

/**
 * \brief Determine whether a node id reference is the end node id reference.
 *
 * \param idref         The 16 byte UUID reference to compare.
 *
 * \returns true if this node id reference is the end node reference (all 0xFF
 * bytes) or false otherwise.
 */
bool dataservice_api_node_ref_is_end(const uint8_t* idref)
{
    MODEL_ASSERT(NULL != idref);
    MODEL_ASSERT(valid_range(idref, 16));

    return !crypto_memcmp(idref, dataservice_api_node_ref_end, 16);
}
