#include <cbmc/model_assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>

bool nondet_bool();

#include "descriptor_hack.h"

int socketpair(int domain, int type, int protocol, int sv[2])
{
    MODEL_ASSERT(AF_UNIX == domain);
    MODEL_ASSERT(SOCK_STREAM == type);
    MODEL_ASSERT(0 == protocol);

    if (nondet_bool())
    {
        if (curr_descriptor < MAX_DESCRIPTORS - 2)
        {
            /* trick CBMC into tracking open descriptors. */
            descriptor_array[curr_descriptor] = (int*)malloc(sizeof(int));

            /* allocation could fail. */
            if (NULL == descriptor_array[curr_descriptor])
                return -1;

            descriptor_array[curr_descriptor + 1] = (int*)malloc(sizeof(int));

            /* allocation could fail. */
            if (NULL == descriptor_array[curr_descriptor + 1])
            {
                free(descriptor_array[curr_descriptor]);
                descriptor_array[curr_descriptor] = NULL;

                return -1;
            }

            /* Set these values to something useful. */
            (*descriptor_array[curr_descriptor]) = curr_descriptor;
            (*descriptor_array[curr_descriptor + 1]) = curr_descriptor + 1;

            /* save the values. */
            sv[0] = curr_descriptor;
            sv[1] = curr_descriptor + 1;

            /* increment curr_descriptor. */
            curr_descriptor += 2;

            /* success. */
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
}
