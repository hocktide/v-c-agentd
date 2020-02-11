#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include "../src/dataservice/dataservice_protocol_internal.h"

uint64_t nondet_block_height();

int main(int argc, char* argv[])
{
    void* payload = NULL;
    size_t payload_size = 0U;

    const uint8_t txn_id[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    const uint8_t prev_id[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    const uint8_t next_id[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    const uint8_t artifact_id[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    const uint8_t block_id[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    const uint8_t cert[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    size_t cert_size = 16;
    uint32_t net_state = htonl(DATASERVICE_TRANSACTION_NODE_STATE_CANONIZED);

    int retval =
        dataservice_encode_response_canonized_transaction_get(
            &payload, &payload_size, txn_id, prev_id, next_id, artifact_id,
            block_id, net_state, cert, cert_size);
    if (AGENTD_STATUS_SUCCESS != retval)
        return 0;

    memset(payload, 0, payload_size);
    free(payload);

    return 0;
}
