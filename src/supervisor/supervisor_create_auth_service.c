/**
 * \file supervisor/supervisor_create_auth_service.c
 *
 * \brief Create the auth service as a process that can be started.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/authservice.h>
#include <agentd/authservice/api.h>
#include <agentd/control.h>
#include <agentd/supervisor/supervisor_internal.h>
#include <agentd/ipc.h>
#include <ctype.h>
#include <vpr/allocator/malloc_allocator.h>

/**
 * \brief Auth service process structure.
 */
typedef struct auth_process
{
    process_t hdr;
    const bootstrap_config_t* bconf;
    const agent_config_t* conf;
    int* auth_socket;
    int* log_socket;
} auth_process_t;

/* forward decls. */
static void supervisor_dispose_auth_service(void* disposable);
static int supervisor_start_auth_service(process_t* proc);
static int supervisor_init_auth_service_read_environment(
    vccrypt_buffer_t*, vccrypt_buffer_t*, vccrypt_buffer_t*);
static int convert_uuid(
    vccrypt_buffer_t*, allocator_options_t*, const char*);
static int convert_hexstring(
    vccrypt_buffer_t*, allocator_options_t*, const char*, size_t);

/**
 * \brief Create the auth service as a process that can be started.
 *
 * \param svc                   Pointer to the pointer to receive the process
 *                              descriptor for the auth service.
 * \param bconf                 Agentd bootstrap config for this service.
 * \param conf                  Agentd configuration to be used to build the
 *                              auth service.  This configuration must be
 *                              valid for the lifetime of the service.
 * \param auth_socket           The auth socket descriptor.
 * \param log_socket            The log socket descriptor.
 *
 * \returns a status indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - a non-zero error code on failure.
 */
int supervisor_create_auth_service(
    process_t** svc, const bootstrap_config_t* bconf,
    const agent_config_t* conf, int* auth_socket, int* log_socket)
{
    int retval;

    /* allocate memory for the auth process. */
    auth_process_t* auth_proc =
        (auth_process_t*)malloc(sizeof(auth_process_t));
    if (NULL == auth_proc)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* set up auth_proc structure. */
    memset(auth_proc, 0, sizeof(auth_process_t));
    auth_proc->hdr.hdr.dispose = &supervisor_dispose_auth_service;
    auth_proc->hdr.init_method = &supervisor_start_auth_service;
    auth_proc->bconf = bconf;
    auth_proc->conf = conf;
    auth_proc->auth_socket = auth_socket;
    auth_proc->log_socket = log_socket;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    *svc = (process_t*)auth_proc;
    goto done;

done:
    return retval;
}

/**
 * \brief Start the auth service.
 *
 * \param proc      The auth service to start.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 */
static int supervisor_start_auth_service(process_t* proc)
{
    auth_process_t* auth_proc = (auth_process_t*)proc;
    uint32_t offset, status;
    int retval;

    vccrypt_buffer_t agent_id_buffer;
    vccrypt_buffer_t agent_pubkey_buffer;
    vccrypt_buffer_t agent_privkey_buffer;

    /* attempt to create the auth service. */
    TRY_OR_FAIL(
        auth_service_proc(
            auth_proc->bconf, auth_proc->conf,
            *auth_proc->log_socket, auth_proc->auth_socket,
            &auth_proc->hdr.process_id, true),
        done);

    /* read the initialization data in from environment variables */
    TRY_OR_FAIL(
        supervisor_init_auth_service_read_environment(
            &agent_id_buffer, &agent_pubkey_buffer, &agent_privkey_buffer),
        terminate_proc);

    /* initialize auth service with agent ID, and public and private keys */
    TRY_OR_FAIL(
        auth_service_api_sendreq_initialize_block(
            *auth_proc->auth_socket, &agent_id_buffer, &agent_pubkey_buffer,
            &agent_privkey_buffer),
        cleanup_buffers);

    TRY_OR_FAIL(
        auth_service_api_recvresp_initialize_block(
            *auth_proc->auth_socket, &offset, &status),
        cleanup_buffers);

    /* verify that the operation completed successfully. */
    TRY_OR_FAIL(status, cleanup_buffers);

    /* if successful, the child process owns the sockets. */
    *auth_proc->log_socket = -1;

    /* success */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

cleanup_buffers:
    dispose((disposable_t*)&agent_privkey_buffer);
    dispose((disposable_t*)&agent_pubkey_buffer);
    dispose((disposable_t*)&agent_id_buffer);

terminate_proc:
    /* force the running status to true so we can terminate the process. */
    auth_proc->hdr.running = true;
    process_stop((process_t*)auth_proc);
    sleep(5);
    process_kill((process_t*)auth_proc);

done:
    return retval;
}

/**
 * \brief Dispose of the auth service by cleaning up.
 */
static void supervisor_dispose_auth_service(void* disposable)
{
    auth_process_t* auth_proc = (auth_process_t*)disposable;

    /* clean up the accept socket if valid. */
    if (*auth_proc->auth_socket > 0)
    {
        close(*auth_proc->auth_socket);
        *auth_proc->auth_socket = -1;
    }

    /* clean up the log socket if valid. */
    if (*auth_proc->log_socket > 0)
    {
        close(*auth_proc->log_socket);
        *auth_proc->log_socket = -1;
    }

    if (auth_proc->hdr.running)
    {
        /* call the process stop method. */
        process_stop((process_t*)auth_proc);

        sleep(5);

        /* kill the process. */
        process_kill((process_t*)auth_proc);
    }
}

/**
 * \brief Read configuration from the environment.
 *
 * Reads the agent ID, agent public key, and agent private key from the 
 * environment and initializes the provided crypto buffers with them.
 * This function should be removed or modified once the key data is read
 * from file.
 *
 * Upon succesful completion of this method, the caller owns the crypto buffers
 * and must dispose of them.
 *
 * \param agent_id_buffer       A crypto buffer to receive the agent ID
 * \param agent_pubkey_buffer   A crypto buffer to receive the agent public key
 * \param agent_privkey_buffer  A crypto buffer to receive the agent priv key
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            environment bootstrapping was not possible.
 */
static int supervisor_init_auth_service_read_environment(
    vccrypt_buffer_t* agent_id_buffer, vccrypt_buffer_t* agent_pubkey_buffer,
    vccrypt_buffer_t* agent_privkey_buffer)
{
    MODEL_ASSERT(NULL != agent_id_buffer);
    MODEL_ASSERT(NULL != agent_pubkey_buffer);
    MODEL_ASSERT(NULL != agent_privkey_buffer);

    int retval = AGENTD_STATUS_SUCCESS;

    allocator_options_t alloc_opts;
    malloc_allocator_options_init(&alloc_opts);

    /* get the agent UUID. */
    const char* agent_id = getenv("AGENTD_ID");
    if (NULL == agent_id)
    {
        agent_id = "cb6c02aa-605f-4f81-bb01-5bb6f5975746";
    }

    /* convert the agent UUID string to a uuid. */
    if (AGENTD_STATUS_SUCCESS !=
        convert_uuid(agent_id_buffer, &alloc_opts, agent_id))
    {
        retval = AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_allocator;
    }

    /* get the agent public key. */
    const char* agent_pubkey = getenv("AGENTD_PUBLIC_KEY");
    if (NULL == agent_pubkey)
    {
        agent_pubkey =
            "de9edb7d7b7dc1b4d35b61c2ece43537"
            "3f8343c85b78674dadfc7e146f882b4f";
    }

    /* convert the agent public key to a buffer. */
    if (AGENTD_STATUS_SUCCESS !=
        convert_hexstring(agent_pubkey_buffer, &alloc_opts, agent_pubkey, 32))
    {
        retval = AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_agent_id_buffer;
    }

    /* get the agent private key. */
    const char* agent_privkey = getenv("AGENTD_PRIVATE_KEY");
    if (NULL == agent_privkey)
    {
        agent_privkey =
            "5dab087e624a8a4b79e17f8b83800ee6"
            "6f3bb1292618b6fd1c2f8b27ff88e0eb";
    }

    /* convert the agent private key to a buffer. */
    if (AGENTD_STATUS_SUCCESS !=
        convert_hexstring(
            agent_privkey_buffer, &alloc_opts, agent_privkey, 32))
    {
        retval = AGENTD_ERROR_AUTHSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_agent_pubkey_buffer;
    }

    /* success */
    goto cleanup_allocator;

cleanup_agent_pubkey_buffer:
    dispose((disposable_t*)agent_pubkey_buffer);

cleanup_agent_id_buffer:
    dispose((disposable_t*)agent_id_buffer);

cleanup_allocator:
    dispose((disposable_t*)&alloc_opts);

    return retval;
}


/**
 * \brief Convert a uuid string to a uuid value.
 *
 * This method attempts to initialize a vccrypt_buffer_t value of the correct
 * size to hold a uuid, and parses a uuid into this buffer.  On success, the
 * caller owns this buffer and must dispose of it.
 *
 * \param uuid_buffer       The buffer to hold the uuid. Initialized by this
 *                          method.  On success, this buffer is owned by the
 *                          caller and must be disposed.
 * \param alloc             The allocator used to allocate the returned buffer.
 * \param uuid_string       The uuid string to parse.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int convert_uuid(
    vccrypt_buffer_t* uuid_buffer, allocator_options_t* alloc,
    const char* uuid_string)
{
    int retval = AGENTD_STATUS_SUCCESS;
    int i = 0;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != uuid_buffer);
    MODEL_ASSERT(NULL != alloc);
    MODEL_ASSERT(NULL != uuid_string);

    /* initialize a hex buffer for conversion. */
    vccrypt_buffer_t hex_buffer;
    if (AGENTD_STATUS_SUCCESS != vccrypt_buffer_init(&hex_buffer, alloc, 32))
    {
        retval = 1;
        goto done;
    }

    /* copy the hex values into the buffer. */
    uint8_t* uuid_out = (uint8_t*)hex_buffer.data;
    for (; *uuid_string != 0 && i < 32; ++uuid_string)
    {
        if (isxdigit(*uuid_string))
        {
            uuid_out[i++] = *uuid_string;
        }
    }

    /* verify that this is a valid uuid string. */
    if (i != 32)
    {
        retval = 2;
        goto cleanup_hex_buffer;
    }

    /* initialize the output buffer for hex conversion. */
    if (AGENTD_STATUS_SUCCESS != vccrypt_buffer_init(uuid_buffer, alloc, 16))
    {
        retval = 3;
        goto cleanup_hex_buffer;
    }

    /* convert the hex data. */
    if (AGENTD_STATUS_SUCCESS !=
        vccrypt_buffer_read_hex(uuid_buffer, &hex_buffer))
    {
        retval = 4;
        goto cleanup_uuid_buffer;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_hex_buffer;

cleanup_uuid_buffer:
    dispose((disposable_t*)uuid_buffer);

cleanup_hex_buffer:
    dispose((disposable_t*)&hex_buffer);

done:
    return retval;
}

/**
 * \brief Convert a hex string to a binary value.
 *
 * This method attempts to initialize a vccrypt_buffer_t value of the correct
 * size to hold the binary value, and parses a hex string into this buffer.  On
 * success, the caller owns this buffer and must dispose of it.
 *
 * \param buffer        The buffer to hold the binary value. Initialized by this
 *                      method.  On success, this buffer is owned by the caller
 *                      and must be disposed.
 * \param alloc         The allocator used to allocate the returned buffer.
 * \param hex           The hex string to parse.
 * \param size          The expected size of this buffer.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int convert_hexstring(
    vccrypt_buffer_t* buffer, allocator_options_t* alloc, const char* hex,
    size_t size)
{
    int retval = AGENTD_STATUS_SUCCESS;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != buffer);
    MODEL_ASSERT(NULL != alloc);
    MODEL_ASSERT(NULL != hex);

    /* get and verify the hex string size. */
    size_t hex_size = strlen(hex);
    if (hex_size % 2 != 0 || hex_size != size * 2)
    {
        retval = 1;
        goto done;
    }

    /* initialize a hex buffer for conversion. */
    vccrypt_buffer_t hex_buffer;
    if (AGENTD_STATUS_SUCCESS !=
        vccrypt_buffer_init(&hex_buffer, alloc, strlen(hex)))
    {
        retval = 2;
        goto done;
    }

    /* copy the hex values into the buffer. */
    memcpy(hex_buffer.data, hex, strlen(hex));

    /* initialize the output buffer for hex conversion. */
    if (AGENTD_STATUS_SUCCESS !=
        vccrypt_buffer_init(buffer, alloc, hex_buffer.size / 2))
    {
        retval = 3;
        goto cleanup_hex_buffer;
    }

    /* convert the hex data. */
    if (AGENTD_STATUS_SUCCESS !=
        vccrypt_buffer_read_hex(buffer, &hex_buffer))
    {
        retval = 4;
        goto cleanup_buffer;
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto cleanup_hex_buffer;

cleanup_buffer:
    dispose((disposable_t*)buffer);

cleanup_hex_buffer:
    dispose((disposable_t*)&hex_buffer);

done:
    return retval;
}
