/**
 * \file agentd/modelcheck.h
 *
 * \brief Helper macros for dealing with model checking.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */
#ifndef AGENTD_MODELCHECK_HEADER_GUARD
#define AGENTD_MODELCHECK_HEADER_GUARD

#include <string.h>

#ifdef CBMC
#define modelsafe_memcpy(a, b, c) \
    do \
    { \
        if ((c) > 0) \
        { \
            uint8_t* ba = (uint8_t*)(a); \
            const uint8_t* bb = (const uint8_t*)(b); \
            ba[0] = bb[0]; \
            ba[(c)-1] = bb[(c)-1]; \
        } \
    } while (0)

#define modelsafe_memset(a, b, c) \
    do \
    { \
        if ((c) > 0) \
        { \
            uint8_t* ba = (uint8_t*)(a); \
            ba[0] = (b); \
            ba[(c)-1] = (b); \
        } \
    } while (0)

#else
#define modelsafe_memcpy memcpy
#define modelsafe_memset memset
#endif /*CBMC*/

#endif /*AGENTD_MODELCHECK_HEADER_GUARD*/
