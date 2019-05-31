/**
 * \file agentd/randomservice/private/randomservice.h
 *
 * \brief Private internal API for the random service.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_RANDOMSERVICE_PRIVATE_RANDOMSERVICE_HEADER_GUARD
#define AGENTD_RANDOMSERVICE_PRIVATE_RANDOMSERVICE_HEADER_GUARD

#include <agentd/ipc.h>
#include <agentd/randomservice.h>
#include <stdbool.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * \brief Context structure for the internal random service.
 */
typedef struct randomservice_root_context
{
    /**
     * \brief This structure is disposable.
     */
    disposable_t hdr;

    /**
     * \brief PRNG descriptor.
     */
    int random_fd;

    /**
     * \brief Loop context.
     */
    ipc_event_loop_context_t* loop_context;

    /**
     * \brief Set to true to force this service to exit.
     */
    bool randomservice_force_exit;

} randomservice_root_context_t;

/**
 * \brief Read random bytes.
 *
 * \param ctx           The private context for this random service.
 * \param buffer        Buffer to hold the random bytes.  Must be initialized.
 *                      Will be filled with random bytes.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - non-zero error code on failure.
 */
int randomservice_read(
    randomservice_root_context_t* ctx,
    vccrypt_buffer_t* buffer);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_RANDOMSERVICE_PRIVATE_RANDOMSERVICE_HEADER_GUARD*/
