#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <stdlib.h>

int dataservice_encode_response_child_context_create(
    void** payload, size_t* payload_size, uint32_t child_offset)
{
    /* parameter sanity check. */
    MODEL_ASSERT(NULL != payload);
    MODEL_ASSERT(NULL != payload_size);

    /* allocate memory for the payload. */
    *payload_size = sizeof(uint32_t);
    *payload = malloc(*payload_size);
    if (NULL == *payload)
    {
        return AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* success. */
    return AGENTD_STATUS_SUCCESS;
}
