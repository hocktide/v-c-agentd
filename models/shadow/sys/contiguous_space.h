#ifndef MODEL_SHADOW_SYS_CONTIGUOUS_SPACE_HEADER_GUARD
#define MODEL_SHADOW_SYS_CONTIGUOUS_SPACE_HEADER_GUARD

#include <stdint.h>

inline bool valid_range(const void* buf, size_t size)
{
    const uint8_t* bbuf = (const uint8_t*)buf;

    return size == 0 || (bbuf[0] == bbuf[0]) && (bbuf[size - 1] == bbuf[size - 1]);
}

#endif /*MODEL_SHADOW_SYS_CONTIGUOUS_SPACE_HEADER_GUARD*/
