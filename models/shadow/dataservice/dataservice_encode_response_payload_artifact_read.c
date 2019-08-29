#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <stdint.h>
#include <stdlib.h>

int dataservice_encode_response_payload_artifact_read(
    void** payload, size_t* payload_size, const uint8_t* artifact_id,
    const uint8_t* txn_first, const uint8_t* txn_latest, uint64_t height_first,
    uint64_t height_latest, uint32_t state_latest)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(NULL != payload_size);
    MODEL_ASSERT(NULL != artifact_id);
    MODEL_ASSERT(NULL != txn_first);
    MODEL_ASSERT(NULL != txn_latest);

    /* create the payload. */
    *payload_size = 3 * 16 + 2 * sizeof(uint64_t) + sizeof(uint32_t);
    *payload = malloc(*payload_size);
    uint8_t* payload_bytes = (uint8_t*)*payload;
    if (NULL == payload_bytes)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
