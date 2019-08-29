#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include "../src/dataservice/dataservice_protocol_internal.h"

int main(int argc, char* argv[])
{
    void* payload = NULL;
    size_t payload_size = 0U;

    const uint8_t block_id[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    int retval =
        dataservice_encode_response_block_id_latest_read(
            &payload, &payload_size, block_id);
    if (AGENTD_STATUS_SUCCESS != retval)
        return 0;

    memset(payload, 0, payload_size);
    free(payload);

    return 0;
}
