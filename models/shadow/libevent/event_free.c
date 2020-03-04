#include <cbmc/model_assert.h>
#include <event.h>
#include <stdlib.h>

void event_free(struct event* ev)
{
    MODEL_ASSERT(NULL != ev);
    free(ev);
}
