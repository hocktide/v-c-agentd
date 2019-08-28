#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include "../src/dataservice/dataservice_internal.h"

/* nondeterministic size. */
uint8_t nondet_size();
uint32_t nondet_method();
uint32_t nondet_offset();
uint32_t nondet_status();

int main(int argc, char* argv[])
{
    ipc_socket_context_t sock;
    size_t size = nondet_size();

    void* data = NULL;

    if (size > 0)
    {
        data = (const void*)malloc(size);
        if (NULL == data)
            return 0;
    }

    dataservice_decode_and_dispatch_write_status(
        &sock, nondet_method(), nondet_offset(), nondet_status(), data, size);

    if (NULL != data)
        free(data);

    return 0;
}
