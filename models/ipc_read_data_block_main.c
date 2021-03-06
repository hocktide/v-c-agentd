#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

int main(int argc, char* argv[])
{
    int lhs, rhs;
    void* val = NULL;
    uint32_t size = 0;

    /* model the creation of a socketpair. */
    int retval =
        ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        /* ipc_socketpair only fails with AGENTD_ERROR_IPC_SOCKETPAIR_FAILURE */
        MODEL_ASSERT(AGENTD_ERROR_IPC_SOCKETPAIR_FAILURE == retval);

        return 0;
    }

    retval =
        ipc_read_data_block(lhs, &val, &size);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        MODEL_ASSERT((AGENTD_ERROR_IPC_READ_BLOCK_FAILURE == retval) || (AGENTD_ERROR_IPC_READ_UNEXPECTED_DATA_TYPE == retval) || (AGENTD_ERROR_GENERAL_OUT_OF_MEMORY == retval));
        close(lhs);
        close(rhs);

        return 0;
    }

    /* clean up memory. */
    free(val);

    /* on success, the lhs and rhs descriptors must be closed. */
    close(lhs);
    close(rhs);

    return 0;
}
