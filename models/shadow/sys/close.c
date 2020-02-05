#include <cbmc/model_assert.h>
#include <stdbool.h>

#include "descriptor_hack.h"

int close(int fd)
{
    MODEL_ASSERT(fd >= 0);
    MODEL_ASSERT(NULL != descriptor_array[fd]);

    /* trick CBMC into tracking release of resource. */
    free(descriptor_array[fd]);

    /* only a single close allowed. */
    descriptor_array[fd] = NULL;
}
