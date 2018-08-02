/**
 * \file inet/htonll.c
 *
 * \brief Convert the given host 64-bit value to network byte order.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>

/**
 * \brief Convert the given host 64-bit value to network byte order.
 *
 * \param val       The value to convert.
 *
 * \returns the converted value.
 */
int64_t htonll(int64_t val)
{
    int64_t hval =
        (val & 0x00000000000000FFUL) << 56 | (val & 0x000000000000FF00UL) << 40 | (val & 0x0000000000FF0000UL) << 24 | (val & 0x00000000FF000000UL) << 8 | (val & 0x000000FF00000000UL) >> 8 | (val & 0x0000FF0000000000UL) >> 24 | (val & 0x00FF000000000000UL) >> 40 | (val & 0xFF00000000000000UL) >> 56;

    return hval;
}
