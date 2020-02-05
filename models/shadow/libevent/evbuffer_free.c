#include <cbmc/model_assert.h>
#include <event.h>
#include <stdlib.h>

void evbuffer_free(struct evbuffer* buf)
{
    MODEL_ASSERT(NULL != buf);
    free(buf);
}
