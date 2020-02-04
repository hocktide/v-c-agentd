#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include "../src/dataservice/dataservice_protocol_internal.h"

/* nondeterministic size. */
uint8_t nondet_size();

int main(int argc, char* argv[])
{
    dataservice_request_global_setting_get_t dreq;
    size_t size = nondet_size();

    const void* req = (const void*)malloc(size);
    if (NULL == req)
        return 0;

    int retval =
        dataservice_decode_request_global_setting_get(req, size, &dreq);
    if (AGENTD_STATUS_SUCCESS == retval)
        dispose((disposable_t*)&dreq);

    free(req);

    return 0;
}
