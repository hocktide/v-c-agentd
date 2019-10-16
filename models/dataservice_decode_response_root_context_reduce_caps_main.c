#include <agentd/dataservice/async_api.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

/* nondeterministic size. */
uint8_t nondet_size();

int main(int argc, char* argv[])
{
    int retval = 0;
    size_t size = nondet_size();
    void* val = malloc(size);
    if (NULL == val)
        return 0;

    /* decode the response. */
    dataservice_response_root_context_reduce_caps_t dresp;
    retval =
        dataservice_decode_response_root_context_reduce_caps(val, size, &dresp);
    if (AGENTD_STATUS_SUCCESS == retval)
    {
        dispose((disposable_t*)&dresp);
    }

    free(val);

    return 0;
}
