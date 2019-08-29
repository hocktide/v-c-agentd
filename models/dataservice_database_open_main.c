#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include "../src/dataservice/dataservice_internal.h"

int main(int argc, char* argv[])
{
    dataservice_root_context_t ctx;

    int retval =
        dataservice_database_open(
            &ctx, "testdir");
    if (AGENTD_STATUS_SUCCESS != retval)
        return 0;

    dataservice_database_close(&ctx);

    return 0;
}
