/**
 * \file agentd/string.h
 *
 * \brief String manipulation functions.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_STRING_HEADER_GUARD
#define AGENTD_STRING_HEADER_GUARD

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Concatenate multiple strings into a single allocated string value.
 *
 * The string value returned is owned by the caller and must be free()d when no
 * longer needed.
 *
 * The parameters are a NULL-terminated list of string values.  Each are
 * concatenated into a single string that is sized large enough to hold data for
 * each of them.  It is critical that the last argument passed to strcatv be
 * NULL.
 *
 * \returns a pointer to a heap allocated string value that is the concatenation
 *          of all input strings.
 */
char* strcatv(const char* string1, ...);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_STRING_HEADER_GUARD*/
