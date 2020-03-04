#include <cbmc/model_assert.h>
#include <stdbool.h>

#include "contiguous_space.h"
#include "descriptor_hack.h"

bool nondet_bool();
uint8_t nondet_byte();
size_t nondet_size();

ssize_t read(int fd, void* buf, size_t count)
{
    MODEL_ASSERT(fd >= 0);
    MODEL_ASSERT(NULL != descriptor_array[fd]);
    MODEL_ASSERT(NULL != buf);

    /* verify the min and max bounds of buf with respect to count. */
    MODEL_ASSERT(valid_range(buf, count));

    /* mutate the buf data. */
    for (int i = 0; i < count; ++i)
    {
        uint8_t* bbuf = (uint8_t*)buf;
        bbuf[i] = nondet_byte();
    }

    /* return an error randomly. */
    if (!nondet_bool())
        return -1;

    /* return a size between 0 and count. */
    size_t val = nondet_size();
    if (val >= count)
    {
        return count;
    }
    else
    {
        return count - val;
    }
}
