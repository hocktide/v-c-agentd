/**
 * \file agentd/bitcap.h
 *
 * \brief Capabilities bitset.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_BITCAP_HEADER_GUARD
#define AGENTD_BITCAP_HEADER_GUARD

#include <cbmc/model_assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * Macro to create a bitcap.
 */
#define BITCAP(x, bitsize) uint32_t x[((bitsize) / 32) + \
    ((bitsize) % 32 > 0 ? 1 : 0)]
/**
 * Helper macro to get the offset.
 */
#define BITCAP_OFFSET(bit) ((bit) / 32)

/**
 * Helper macro to get the shift.
 */
#define BITCAP_SHIFT(bit) ((bit) % 32)

/**
 * Macro to initialize a bitcap with all falses.
 */
#define BITCAP_INIT_FALSE(x) memset((x), 0, sizeof((x)))

/**
 * Macro to initialize a bitcap with all trues.
 */
#define BITCAP_INIT_TRUE(x) memset((x), 0xFF, sizeof((x)))

/**
 * Macro to set a particular bit to true.
 */
#define BITCAP_SET_TRUE(cap, bit) \
    (cap)[BITCAP_OFFSET(bit)] |= (1 << BITCAP_SHIFT(bit))

/**
 * Macro to set a particular bit to false.
 */
#define BITCAP_SET_FALSE(cap, bit) \
    (cap)[BITCAP_OFFSET(bit)] &= ~(1 << BITCAP_SHIFT(bit))

/**
 * Macro returns true if a given bit is set.
 */
#define BITCAP_ISSET(cap, bit) \
    (bool)(((cap)[BITCAP_OFFSET(bit)] & (1 << BITCAP_SHIFT(bit))) != 0U)

/**
 * Macro to create an intersection of two bitsets.  C = A & B
 */
#define BITCAP_INTERSECT(C, A, B) \
    do \
    { \
        MODEL_ASSERT(sizeof((C)) == sizeof((A)) && sizeof((C)) == sizeof((B))); \
        for (size_t ibitcap = 0; ibitcap < sizeof((C)) / sizeof(uint32_t); \
             ++ibitcap) \
        { \
            (C)[ibitcap] = (A)[ibitcap] & (B)[ibitcap]; \
        } \
    } while (false)

/**
 * Macro to create a union of two bitsets.  C = A | B
 */
#define BITCAP_UNION(C, A, B) \
    do \
    { \
        MODEL_ASSERT(sizeof((C)) == sizeof((A)) && sizeof((C)) == sizeof((B))); \
        for (size_t ibitcap = 0; ibitcap < sizeof((C)) / sizeof(uint32_t); \
             ++ibitcap) \
        { \
            (C)[ibitcap] = (A)[ibitcap] | (B)[ibitcap]; \
        } \
    } while (false)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_BITCAP_HEADER_GUARD*/
