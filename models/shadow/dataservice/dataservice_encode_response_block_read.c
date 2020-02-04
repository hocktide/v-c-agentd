#include <agentd/inet.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <stdlib.h>

int dataservice_encode_response_block_read(
    void** payload, size_t* payload_size, const uint8_t* block_id,
    const uint8_t* prev_id, const uint8_t* next_id, const uint8_t* first_txn_id,
    uint64_t block_height, const void* cert, size_t cert_size)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(NULL != payload_size);
    MODEL_ASSERT(NULL != block_id);
    MODEL_ASSERT(NULL != prev_id);
    MODEL_ASSERT(NULL != next_id);
    MODEL_ASSERT(NULL != first_txn_id);
    MODEL_ASSERT(NULL != cert);

    /* create the payload. */
    *payload_size = 4 * 16 + sizeof(uint64_t) + cert_size;
    *payload = (uint8_t*)malloc(*payload_size);
    uint8_t* payload_bytes = (uint8_t*)*payload;
    if (NULL == payload_bytes)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
