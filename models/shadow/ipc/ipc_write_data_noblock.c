#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

int nondet_int();

/**
 * \brief Write a raw data packet to a non-blocking socket.
 *
 * On success, the raw data packet value will be written, along with type
 * information and size.
 *
 * \param sd            The socket descriptor to which the value is written.
 * \param val           The raw data to write.
 * \param size          The size of the raw data to write.
 *
 * \returns A status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE if adding the type
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE if adding the size
 *        data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE if adding the
 *        payload data to the write buffer failed.
 *      - AGENTD_ERROR_IPC_WRITE_NONBLOCK_FAILURE if a non-blocking write
 *        failed.
 */
int ipc_write_data_noblock(
    ipc_socket_context_t* sock, const void* val, uint32_t size)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != sock);
    MODEL_ASSERT(NULL != val);

    switch (nondet_int())
    {
        case 0:
            return AGENTD_ERROR_IPC_WRITE_BUFFER_TYPE_ADD_FAILURE;
        case 1:
            return AGENTD_ERROR_IPC_WRITE_BUFFER_SIZE_ADD_FAILURE;
        case 2:
            return AGENTD_ERROR_IPC_WRITE_BUFFER_PAYLOAD_ADD_FAILURE;
        case 3:
            return AGENTD_ERROR_IPC_WRITE_NONBLOCK_FAILURE;
        case 4:
            return AGENTD_ERROR_IPC_WOULD_BLOCK;
        default:
            return AGENTD_STATUS_SUCCESS;
    }
}
