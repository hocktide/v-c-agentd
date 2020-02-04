#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include "../src/dataservice/dataservice_protocol_internal.h"

uint64_t nondet_child_offset();

int main(int argc, char* argv[])
{
    void* payload = NULL;
    size_t payload_size = 0U;

    int retval =
        dataservice_encode_response_child_context_create(
            &payload, &payload_size, nondet_child_offset);
    if (AGENTD_STATUS_SUCCESS != retval)
        return 0;

    memset(payload, 0, payload_size);
    free(payload);

    return 0;
}
