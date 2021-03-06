#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

int main(int argc, char* argv[])
{
    int lhs, rhs;

    /* model the creation of a socketpair. */
    int retval =
        ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        /* ipc_socketpair only fails with AGENTD_ERROR_IPC_SOCKETPAIR_FAILURE */
        MODEL_ASSERT(AGENTD_ERROR_IPC_SOCKETPAIR_FAILURE == retval);

        return 0;
    }

    const char* str = "test";

    retval =
        ipc_write_string_block(lhs, str);
    if (AGENTD_STATUS_SUCCESS != retval)
    {
        MODEL_ASSERT(AGENTD_ERROR_IPC_WRITE_BLOCK_FAILURE == retval);
    }

    /* on success, the lhs and rhs descriptors must be closed. */
    close(lhs);
    close(rhs);

    return 0;
}
