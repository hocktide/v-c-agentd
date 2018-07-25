/**
 * \file string/strcatv.c
 *
 * \brief Variable length strcat function.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

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
char* strcatv(const char* string1, ...)
{
    va_list str_list;
    va_list count_list;
    const char* arg = string1;
    size_t size = 1U;

    va_start(str_list, string1);
    va_copy(count_list, str_list);

    /* get the total size. */
    while (NULL != arg)
    {
        size += strlen(arg);
        arg = va_arg(count_list, const char*);
    }

    /* allocate size for the copied string. */
    char* outstr = (char*)malloc(size);
    if (NULL == outstr)
    {
        return NULL;
    }

    /* copy all strings into this string. */
    size_t offset = 0U;
    arg = string1;
    while (NULL != arg)
    {
        size_t argsize = strlen(arg);
        memcpy(outstr + offset, arg, argsize);
        offset += argsize;
        arg = va_arg(str_list, const char*);
    }

    outstr[offset] = 0;

    return outstr;
}
