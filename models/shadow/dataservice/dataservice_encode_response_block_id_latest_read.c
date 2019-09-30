#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <stdlib.h>

int dataservice_encode_response_block_id_latest_read(
    void** payload, size_t* payload_size, const uint8_t* block_id)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(NULL != payload_size);
    MODEL_ASSERT(NULL != block_id);

    /* create the payload. */
    *payload_size = 16U;
    *payload = malloc(*payload_size);
    if (NULL == *payload)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    return AGENTD_STATUS_SUCCESS;
}
