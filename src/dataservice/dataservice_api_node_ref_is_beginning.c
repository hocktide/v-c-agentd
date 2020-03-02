/**
 * \file dataservice/dataservice_api_node_ref_is_beginning.c
 *
 * \brief Determine whether a node id reference is the beginning node id
 * reference.
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

static const uint8_t dataservice_api_node_ref_beginning[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/**
 * \brief Determine whether a node id reference is the beginning node id
 * reference.
 *
 * \param idref         The 16 byte UUID reference to compare.
 *
 * \returns true if this node id reference is the beginning node reference (all
 * zeroes) or false otherwise.
 */
bool dataservice_api_node_ref_is_beginning(const uint8_t* idref)
{
    MODEL_ASSERT(NULL != idref);
    MODEL_ASSERT(valid_range(idref, 16));

    return !crypto_memcmp(idref, dataservice_api_node_ref_beginning, 16);
}
