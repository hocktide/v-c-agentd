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
        return 0;

    /* model check for ipc_make_block. */
    retval =
        ipc_make_block(lhs);
    MODEL_ASSERT(
        ((AGENTD_ERROR_IPC_FCNTL_GETFL_FAILURE == retval) || (AGENTD_ERROR_IPC_FCNTL_SETFL_FAILURE == retval) || (AGENTD_STATUS_SUCCESS == retval)));

    /* the lhs and rhs descriptors must be closed. */
    close(lhs);
    close(rhs);
}
