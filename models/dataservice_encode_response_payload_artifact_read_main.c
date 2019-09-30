#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include "../src/dataservice/dataservice_protocol_internal.h"

/* nondeterministic size. */
uint64_t nondet_height_first();
uint64_t nondet_height_latest();
uint32_t nondet_state_latest();

int main(int argc, char* argv[])
{
    void* payload = NULL;
    size_t payload_size = 0U;

    const uint8_t artifact_id[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    const uint8_t txn_first[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    const uint8_t txn_latest[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    int retval =
        dataservice_encode_response_payload_artifact_read(
            &payload, &payload_size, artifact_id, txn_first, txn_latest,
            nondet_height_first(), nondet_height_latest(),
            nondet_state_latest());
    if (AGENTD_STATUS_SUCCESS != retval)
        return 0;

    memset(payload, 0, payload_size);
    free(payload);

    return 0;
}
