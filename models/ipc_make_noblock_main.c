#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>

int main(int argc, char* argv[])
{
    int lhs, rhs;
    ipc_socket_context_t sock;

    /* model the creation of a socketpair. */
    int retval =
        ipc_socketpair(AF_UNIX, SOCK_STREAM, 0, &lhs, &rhs);
    if (AGENTD_STATUS_SUCCESS != retval)
        return 0;

    /* model check for ipc_make_noblock. */
    retval =
        ipc_make_noblock(lhs, &sock, NULL);
    MODEL_ASSERT(
        ((AGENTD_ERROR_IPC_FCNTL_GETFL_FAILURE == retval) || (AGENTD_ERROR_IPC_FCNTL_SETFL_FAILURE == retval) || (AGENTD_ERROR_GENERAL_OUT_OF_MEMORY == retval) || (AGENTD_STATUS_SUCCESS == retval)));
    if (AGENTD_STATUS_SUCCESS != retval)
        goto cleanup_sockets;

    dispose((disposable_t*)&sock);
    close(rhs);
    return 0;

cleanup_sockets:
    /* the lhs and rhs descriptors must be closed. */
    close(lhs);
    close(rhs);

    return 0;
}
