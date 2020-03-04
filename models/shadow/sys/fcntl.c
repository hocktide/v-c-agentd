#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>

#include "descriptor_hack.h"

bool nondet_bool();

int fcntl(int fd, int cmd, ... /* arg */)
{
    int retval;
    int arg;
    va_list args;

    MODEL_ASSERT(fd >= 0);
    MODEL_ASSERT(NULL != descriptor_array[fd]);

    va_start(args, cmd);

    /* handle error case. */
    if (!nondet_bool())
    {
        retval = -1;
        goto cleanup;
    }

    /* TODO - verify arg for SETFL. */
    switch (cmd)
    {
        case F_GETFL:
            break;

        case F_SETFL:
            arg = va_arg(args, int);
            break;

        default:
            MODEL_ASSERT(false && "Only F_GETFL and F_SETFL supported.");
            break;
    }

    /* success. */
    retval = 0;

cleanup:
    va_end(args);

    return retval;
}
