/**
 * \file agentd/inet.h
 *
 * \brief Internet routines.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_INET_HEADER_GUARD
#define AGENTD_INET_HEADER_GUARD

#include <stdint.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Convert the given host 64-bit value to network byte order.
 *
 * \param val       The value to convert.
 *
 * \returns the converted value.
 */
int64_t htonll(int64_t val);

/**
 * \brief Convert the given network 64-bit value to host byte order.
 *
 * \param val       The value to convert.
 *
 * \returns the converted value.
 */
#define ntohll(x) htonll((x))

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_IPC_HEADER_GUARD*/
