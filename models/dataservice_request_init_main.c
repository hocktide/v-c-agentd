#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include "../src/dataservice/dataservice_protocol_internal.h"

/* nondeterministic size. */
uint8_t nondet_size();

int main(int argc, char* argv[])
{
    dataservice_request_header_t dreq;
    size_t size = nondet_size();

    const uint8_t* breq0 = (const uint8_t*)malloc(size);
    if (NULL == breq0)
        return 0;

    const uint8_t* breq = breq0;

    int retval =
        dataservice_request_init(&breq, &size, &dreq, sizeof(dreq));
    if (AGENTD_STATUS_SUCCESS == retval)
    {
        MODEL_ASSERT(NULL != dreq.hdr.dispose);
        MODEL_ASSERT(sizeof(dreq) == dreq.size);
        dispose((disposable_t*)&dreq);
    }

    free(breq0);

    return 0;
}
