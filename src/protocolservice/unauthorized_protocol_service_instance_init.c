/**
 * \file protocolservice/unauthorized_protocol_service_instance_init.c
 *
 * \brief Initialize the unauthorized protocol service instance.
 *
 * \copyright 2019 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/ipc.h>
#include <agentd/status_codes.h>
#include <cbmc/model_assert.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <vpr/allocator/malloc_allocator.h>
#include <vpr/parameters.h>

#include "unauthorized_protocol_service_private.h"

/* forward decls */
static void unauthorized_protocol_service_instance_dispose(void* disposable);
static int unauthorized_protocol_service_instance_init_read_environment(
    unauthorized_protocol_service_instance_t* inst);
static int convert_uuid(
    vccrypt_buffer_t*, allocator_options_t*, const char*);
static int convert_hexstring(
    vccrypt_buffer_t*, allocator_options_t*, const char*, size_t);

/**
 * \brief Create the unauthorized protocol service instance.
 *
 * \param inst          The service instance to initialize.
 * \param random        The random socket to use for this instance.
 * \param data          The dataservice socket to use for this instance.
 * \param proto         The protocol socket to use for this instance.
 * \param max_socks     The maximum number of socket connections to accept.
 *
 * \returns a status code indicating success or failure.
 */
int unauthorized_protocol_service_instance_init(
    unauthorized_protocol_service_instance_t* inst, int random, int data,
    int proto, size_t max_socks)
{
    int retval = AGENTD_STATUS_SUCCESS;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != inst);
    MODEL_ASSERT(proto >= 0);
    MODEL_ASSERT(random >= 0);
    MODEL_ASSERT(data >= 0);
    MODEL_ASSERT(max_socks > 0);

    /* Set up the instance basics. */
    memset(inst, 0, sizeof(unauthorized_protocol_service_instance_t));
    inst->hdr.dispose = &unauthorized_protocol_service_instance_dispose;

    /* create the allocator for this instance. */
    malloc_allocator_options_init(&inst->alloc_opts);

    /* create the crypto suite for this instance. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_suite_options_init(
            &inst->suite, &inst->alloc_opts, VCCRYPT_SUITE_VELO_V1))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto done;
    }

    /* create agent pubkey buffer. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_suite_buffer_init_for_auth_key_agreement_public_key(
            &inst->suite, &inst->agent_pubkey))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_suite;
    }

    /* create agent privkey buffer. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_suite_buffer_init_for_auth_key_agreement_private_key(
            &inst->suite, &inst->agent_privkey))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_agent_pubkey_buffer;
    }

    /* create authorized entity pubkey. */
    if (VCCRYPT_STATUS_SUCCESS !=
        vccrypt_suite_buffer_init_for_auth_key_agreement_public_key(
            &inst->suite, &inst->authorized_entity_pubkey))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_agent_privkey_buffer;
    }

    /* read environment data as a temporary hack. */
    /* TODO - replace with config when we can integrate with block tool. */
    if (AGENTD_STATUS_SUCCESS !=
        unauthorized_protocol_service_instance_init_read_environment(
            inst))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_authorized_entity_pubkey_buffer;
    }

    /* set the protocol socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS != ipc_make_noblock(proto, &inst->proto, inst))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_authorized_entity_pubkey_buffer;
    }

    /* set the random socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS != ipc_make_noblock(random, &inst->random, inst))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_proto;
    }

    /* set the data socket to non-blocking. */
    if (AGENTD_STATUS_SUCCESS != ipc_make_noblock(data, &inst->data, inst))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_random;
    }

    /* initialize the IPC event loop instance. */
    if (AGENTD_STATUS_SUCCESS != ipc_event_loop_init(&inst->loop))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_MAKE_NOBLOCK_FAILURE;
        goto cleanup_data;
    }

    /* on these signals, leave the event loop and shut down gracefully. */
    ipc_exit_loop_on_signal(&inst->loop, SIGHUP);
    ipc_exit_loop_on_signal(&inst->loop, SIGTERM);
    ipc_exit_loop_on_signal(&inst->loop, SIGQUIT);

    /* create a single dynamic array and size for all connections so that
     * we can reference them by offset in constant time.
     */
    inst->num_connections = max_socks;
    inst->connections = (unauthorized_protocol_connection_t*)
        malloc(max_socks * sizeof(unauthorized_protocol_connection_t));
    if (NULL == inst->connections)
    {
        retval = AGENTD_ERROR_GENERAL_OUT_OF_MEMORY;
        goto cleanup_loop;
    }

    /* clear all connections */
    memset(
        inst->connections, 0,
        max_socks * sizeof(unauthorized_protocol_connection_t));

    /* move connections to free list. */
    for (size_t i = 0; i < max_socks; ++i)
    {
        /* add this connection to the free list. */
        unauthorized_protocol_connection_push_front(
            &inst->free_connection_head, inst->connections + i);
    }

    /* success. */
    retval = AGENTD_STATUS_SUCCESS;
    goto done;

    /*cleanup_connections:
    free(inst->connections); */

cleanup_loop:
    dispose((disposable_t*)&inst->loop);

cleanup_data:
    dispose((disposable_t*)&inst->data);

cleanup_random:
    dispose((disposable_t*)&inst->random);

cleanup_proto:
    dispose((disposable_t*)&inst->proto);

cleanup_authorized_entity_pubkey_buffer:
    dispose((disposable_t*)&inst->authorized_entity_pubkey);

cleanup_agent_privkey_buffer:
    dispose((disposable_t*)&inst->agent_privkey);

cleanup_agent_pubkey_buffer:
    dispose((disposable_t*)&inst->agent_pubkey);

cleanup_suite:
    dispose((disposable_t*)&inst->suite);

done:
    return retval;
}

/**
 * \brief Dispose of an unauthorized protocol service instance.
 */
static void unauthorized_protocol_service_instance_dispose(void* disposable)
{
    unauthorized_protocol_service_instance_t* inst =
        (unauthorized_protocol_service_instance_t*)disposable;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != inst);

    /* dispose of connections waiting for a free dataservice context. */
    for (unauthorized_protocol_connection_t* i =
             inst->dataservice_context_create_head;
         i != NULL;)
    {
        unauthorized_protocol_connection_t* next = i->next;
        dispose((disposable_t*)i);
        i = next;
    }

    /* dispose of used conections. */
    for (unauthorized_protocol_connection_t* i = inst->used_connection_head;
         i != NULL;)
    {
        unauthorized_protocol_connection_t* next = i->next;
        dispose((disposable_t*)i);
        i = next;
    }

    /* free connection array. */
    memset(
        inst->connections, 0,
        inst->num_connections * sizeof(unauthorized_protocol_connection_t));
    free(inst->connections);

    /* dispose of the proto socket. */
    dispose((disposable_t*)&inst->proto);

    /* dispose of the random socket. */
    dispose((disposable_t*)&inst->random);

    /* dispose of the data socket. */
    dispose((disposable_t*)&inst->data);

    /* dispose of the loop. */
    dispose((disposable_t*)&inst->loop);

    /* dispose of crypto buffers. */
    dispose((disposable_t*)&inst->authorized_entity_pubkey);
    dispose((disposable_t*)&inst->agent_privkey);
    dispose((disposable_t*)&inst->agent_pubkey);

    /* dispose of the crypto suite. */
    dispose((disposable_t*)&inst->suite);

    /* dispose of the allocator. */
    dispose((disposable_t*)&inst->alloc_opts);

    /* clear this instance. */
    memset(inst, 0, sizeof(unauthorized_protocol_service_instance_t));
}

/**
 * \brief Read configuration from the environment.
 *
 * This is a test harness function while we are bootstrapping config.  It should
 * be removed once the private key integration work is done.
 *
 * \param inst          The service instance to initialize with environment
 *                      settings.
 *
 * \returns a status code indicating success or failure.
 *          - AGENTD_STATUS_SUCCESS on success.
 *          - AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE if
 *            environment bootstrapping was not possible.
 */
static int unauthorized_protocol_service_instance_init_read_environment(
    unauthorized_protocol_service_instance_t* inst)
{
    int retval = AGENTD_STATUS_SUCCESS;
    MODEL_ASSERT(NULL != inst);

    /* create a malloc allocator. */
    allocator_options_t alloc;
    malloc_allocator_options_init(&alloc);

    /* get the agent UUID. */
    const char* agent_id = getenv("AGENTD_ID");
    if (NULL == agent_id)
    {
        agent_id = "cb6c02aa-605f-4f81-bb01-5bb6f5975746";
    }

    /* convert the agent UUID string to a uuid. */
    vccrypt_buffer_t agent_id_buffer;
    if (AGENTD_STATUS_SUCCESS !=
        convert_uuid(&agent_id_buffer, &alloc, agent_id))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
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
    vccrypt_buffer_t agent_pubkey_buffer;
    if (AGENTD_STATUS_SUCCESS !=
        convert_hexstring(&agent_pubkey_buffer, &alloc, agent_pubkey, 32))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
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
    vccrypt_buffer_t agent_privkey_buffer;
    if (AGENTD_STATUS_SUCCESS !=
        convert_hexstring(
            &agent_privkey_buffer, &alloc, agent_privkey, 32))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_agent_pubkey_buffer;
    }

    /* get the authorized entity UUID. */
    const char* authorized_entity_id = getenv("AGENTD_AUTHORIZED_ENTITY_ID");
    if (NULL == authorized_entity_id)
    {
        authorized_entity_id = "aca029b6-2602-4b20-a8a4-cd8a95985a9a";
    }

    /* convert the authorized entity UUID string to a uuid. */
    vccrypt_buffer_t authorized_entity_id_buffer;
    if (AGENTD_STATUS_SUCCESS !=
        convert_uuid(
            &authorized_entity_id_buffer, &alloc, authorized_entity_id))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_agent_privkey_buffer;
    }

    /* get the authorized entity public key. */
    const char* authorized_entity_pubkey =
        getenv("AGENTD_AUTHORIZED_ENTITY_PUBKEY");
    if (NULL == authorized_entity_pubkey)
    {
        authorized_entity_pubkey =
            "8520f0098930a754748b7ddcb43ef75a"
            "0dbf3a0d26381af4eba4a98eaa9b4e6a";
    }

    /* convert the authorized entity public key to a buffer. */
    vccrypt_buffer_t authorized_entity_pubkey_buffer;
    if (AGENTD_STATUS_SUCCESS !=
        convert_hexstring(
            &authorized_entity_pubkey_buffer, &alloc,
            authorized_entity_pubkey, 32))
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_authorized_entity_id_buffer;
    }

    /* Copy the agent id. */
    MODEL_ASSERT(agent_id_buffer.size >= sizeof(inst->agent_id));
    memcpy(inst->agent_id, agent_id_buffer.data, sizeof(inst->agent_id));

    /* Copy the agent public key. */
    if (agent_pubkey_buffer.size != inst->agent_pubkey.size)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_authorized_entity_pubkey_buffer;
    }
    MODEL_ASSERT(agent_pubkey_buffer.size == inst->agent_pubkey.size);
    memcpy(
        inst->agent_pubkey.data, agent_pubkey_buffer.data,
        inst->agent_pubkey.size);

    /* Copy the agent private key. */
    if (agent_privkey_buffer.size != inst->agent_privkey.size)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_authorized_entity_pubkey_buffer;
    }
    MODEL_ASSERT(agent_privkey_buffer.size == inst->agent_privkey.size);
    memcpy(
        inst->agent_privkey.data, agent_privkey_buffer.data,
        inst->agent_privkey.size);

    /* Copy the authorized entity id. */
    MODEL_ASSERT(authorized_entity_id_buffer.size >= sizeof(inst->authorized_entity_id));
    memcpy(inst->authorized_entity_id, authorized_entity_id_buffer.data,
        sizeof(inst->authorized_entity_id));

    /* Copy the authorized entity public key. */
    if (authorized_entity_pubkey_buffer.size != inst->authorized_entity_pubkey.size)
    {
        retval = AGENTD_ERROR_PROTOCOLSERVICE_IPC_EVENT_LOOP_INIT_FAILURE;
        goto cleanup_authorized_entity_pubkey_buffer;
    }
    MODEL_ASSERT(authorized_entity_pubkey_buffer.size == inst->authorized_entity_pubkey.size);
    memcpy(inst->authorized_entity_pubkey.data,
        authorized_entity_pubkey_buffer.data,
        inst->authorized_entity_pubkey.size);

    /* success */
    retval = AGENTD_STATUS_SUCCESS;

cleanup_authorized_entity_pubkey_buffer:
    dispose((disposable_t*)&authorized_entity_pubkey_buffer);

cleanup_authorized_entity_id_buffer:
    dispose((disposable_t*)&authorized_entity_id_buffer);

cleanup_agent_privkey_buffer:
    dispose((disposable_t*)&agent_privkey_buffer);

cleanup_agent_pubkey_buffer:
    dispose((disposable_t*)&agent_pubkey_buffer);

cleanup_agent_id_buffer:
    dispose((disposable_t*)&agent_id_buffer);

cleanup_allocator:
    dispose((disposable_t*)&alloc);

    (void)inst;
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
