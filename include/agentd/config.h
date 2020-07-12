/**
 * \file agentd/config.h
 *
 * \brief Configuration data structure for agentd.
 *
 * \copyright 2018-2020 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_CONFIG_HEADER_GUARD
#define AGENTD_CONFIG_HEADER_GUARD

#include <arpa/inet.h>
#include <agentd/bootstrap_config.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vpr/disposable.h>
#include <vpr/uuid.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

#define MATERIALIZED_VIEW_CRUD_CREATE 0x0001
#define MATERIALIZED_VIEW_CRUD_UPDATE 0x0002
#define MATERIALIZED_VIEW_CRUD_APPEND 0x0004
#define MATERIALIZED_VIEW_CRUD_DELETE 0x0008

/**
 * \brief Forward declaration of bootstrap config.
 */
struct bootstrap_config;

/**
 * \brief A config list node is a single-linked list node that provides support
 * for multiple entries.
 */
typedef struct config_list_node
{
    struct config_list_node* next;
} config_list_node_t;

/**
 * \brief Listen address and port.
 */
typedef struct config_listen_address
{
    /** \brief This is a list node. */
    config_list_node_t hdr;
    struct in_addr* addr;
    in_port_t port;
} config_listen_address_t;

/**
 * \brief User ID and group ID.
 */
typedef struct config_user_group
{
    const char* user;
    const char* group;
} config_user_group_t;

/**
 * \brief Canonization data.
 */
typedef struct config_canonization
{
    disposable_t hdr;
    bool block_max_milliseconds_set;
    int64_t block_max_milliseconds;
    bool block_max_transactions_set;
    int64_t block_max_transactions;
} config_canonization_t;

/**
 * \brief Disposable list node.
 */
typedef struct config_disposable_list_node
{
    disposable_t hdr;
    struct config_disposable_list_node* next;
} config_disposable_list_node_t;

/**
 * \brief Materialized field type.
 */
typedef struct config_materialized_field_type
{
    config_disposable_list_node_t hdr;
    vpr_uuid field_code;
    uint16_t short_code;
    uint32_t field_crud_flags;
} config_materialized_field_type_t;

/**
 * \brief Materialized transaction type.
 */
typedef struct config_materialized_transaction_type
{
    config_disposable_list_node_t hdr;
    vpr_uuid transaction_type;
    uint32_t artifact_crud_flags;
    config_materialized_field_type_t* field_head;
} config_materialized_transaction_type_t;

/**
 * \brief Materialized artifact type.
 */
typedef struct config_materialized_artifact_type
{
    config_disposable_list_node_t hdr;
    vpr_uuid artifact_type;
    config_materialized_transaction_type_t* transaction_head;
} config_materialized_artifact_type_t;

/**
 * \brief Materialized view data.
 */
typedef struct config_materialized_view
{
    config_disposable_list_node_t hdr;
    const char* name;
    config_materialized_artifact_type_t* artifact_head;
} config_materialized_view_t;

#define CONFIG_STREAM_TYPE_BOM 0x00
#define CONFIG_STREAM_TYPE_LOGDIR 0x01
#define CONFIG_STREAM_TYPE_LOGLEVEL 0x02
#define CONFIG_STREAM_TYPE_SECRET 0x03
#define CONFIG_STREAM_TYPE_ROOTBLOCK 0x04
#define CONFIG_STREAM_TYPE_DATASTORE 0x05
#define CONFIG_STREAM_TYPE_LISTEN_ADDR 0x06
#define CONFIG_STREAM_TYPE_CHROOT 0x07
#define CONFIG_STREAM_TYPE_USERGROUP 0x08
#define CONFIG_STREAM_TYPE_BLOCK_MAX_MILLISECONDS 0x09
#define CONFIG_STREAM_TYPE_BLOCK_MAX_TRANSACTIONS 0x0A
#define CONFIG_STREAM_TYPE_EOM 0x80
#define CONFIG_STREAM_TYPE_ERROR 0xFF

#define BLOCK_MILLISECONDS_MAXIMUM 43200000
#define BLOCK_TRANSACTIONS_MAXIMUM 100000
/**
 * \brief Root of the agent configuration AST.
 */
typedef struct agent_config
{
    disposable_t hdr;
    const char* logdir;
    bool loglevel_set;
    int64_t loglevel;
    bool block_max_milliseconds_set;
    int64_t block_max_milliseconds;
    bool block_max_transactions_set;
    int64_t block_max_transactions;
    const char* secret;
    const char* rootblock;
    const char* datastore;
    config_listen_address_t* listen_head;
    const char* chroot;
    config_user_group_t* usergroup;
    config_materialized_view_t* view_head;
} agent_config_t;

/**
 * \brief Union for parser.
 */
typedef union config_val
{
    int64_t number;
    vpr_uuid id;
    const char* string;
    struct in_addr* addr;
    agent_config_t* config;
    config_user_group_t* usergroup;
    config_listen_address_t* listenaddr;
    config_canonization_t* canonization;
    config_materialized_view_t* view;
    config_materialized_artifact_type_t* view_artifact;
    config_materialized_transaction_type_t* view_transaction;
    config_materialized_field_type_t* view_field;
} config_val_t;

/* forward decl for config_context. */
struct config_context;

/**
 * \brief This callback provides a means for an error to be set by the caller.
 */
typedef void (*config_set_error_t)(struct config_context*, const char*);

/** 
 * \brief Configuration value callback.
 */
typedef void (*config_val_callback_t)(struct config_context*, agent_config_t*);

/**
 * \brief The config context structure is used to provide user overrides for
 * methods and a user context pointer to the parser.
 */
typedef struct config_context
{
    config_set_error_t set_error;
    config_val_callback_t val_callback;
    void* user_context;
} config_context_t;

/* helper to link our value to Bison. */
#define YYSTYPE config_val_t
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif /*YY_TYPEDEF_YY_SCANNER_T*/

/**
 * \brief Set the scanner to read from a Unix file descriptor.
 *
 * \param scanner       The scanner context structure.
 * \param fd            The file descriptor to use.
 * \param state         The buffer state structure used by the scanner.
 *
 * \returns the minted file handle, to be closed by the caller when scanning
 * is complete.
 */
FILE* config_set_input_filedescriptor(
    yyscan_t scanner, int fd, void* state);

/**
 * \brief Internal method for disposing a config structure.  Do not call.
 */
void config_dispose(void* disp);

/**
 * \brief Write a config structure to a blocking stream.
 *
 * \param s             The socket descriptor to write.
 * \param conf          The config structure to write.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_IPC_WRITE_DATA_FAILURE if writing data to the
 *        socket failed.
 *      - AGENTD_ERROR_CONFIG_INET_NTOP_FAILURE if converting the listen address
 *        to a string failed.
 */
int config_write_block(int s, agent_config_t* conf);

/**
 * \brief Initialize and read an agent config structure from a blocking stream.
 *
 * On success, a config structure is initialized with data from the blocking
 * stream.  This is owned by the caller and must be disposed by calling \ref
 * dispose() when no longer needed.
 *
 * \param s             The socket descriptor to read.
 * \param conf          The config structure to populate.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_GENERAL_OUT_OF_MEMORY if an out-of-memory condition was
 *        encountered during this operation.
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if there was a failure
 *        reading from the config socket.
 *      - AGENTD_ERROR_CONFIG_INVALID_STREAM the stream data was corrupted or
 *        invalid.
 *      - AGENTD_ERROR_CONFIG_INET_PTON_FAILURE if converting an address to a
 *        network address failed.
 */
int config_read_block(int s, agent_config_t* conf);

/**
 * \brief Set default values for any config setting that has not been set.
 *
 * \param conf          The config structure to populate.
 * \param bconf         The bootstrap config structure to use for overrides.
 *
 * \returns 0 on success and non-zero on failure.
 */
int config_set_defaults(agent_config_t* conf, const bootstrap_config_t* bconf);

/**
 * \brief Spawn a process to read config data, populating the provided config
 * structure.
 *
 * On success, a config structure is initialized with data from the config
 * reader process.  This is owned by the caller and must be disposed by calling
 * \ref dispose() when no longer needed.
 *
 * \param bconf         The bootstrap configuration used to spawn the config
 *                      reader process.
 * \param conf          The config structure to populate.
 *
 * \returns a status code indicating success or failure.
 *      - AGENTD_STATUS_SUCCESS on success.
 *      - AGENTD_ERROR_CONFIG_PROC_RUNSECURE_ROOT_USER_REQUIRED if spawning this
 *        process failed because the user is not root and runsecure is true.
 *      - AGENTD_ERROR_CONFIG_IPC_SOCKETPAIR_FAILURE if creating a socketpair
 *        for the dataservice process failed.
 *      - AGENTD_ERROR_CONFIG_FORK_FAILURE if forking the private process
 *        failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_LOOKUP_USERGROUP_FAILURE if there was a
 *        failure looking up the configured user and group for the dataservice
 *        process.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_CHROOT_FAILURE if chrooting failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_DROP_PRIVILEGES_FAILURE if dropping
 *        privileges failed.
 *      - AGENTD_ERROR_CONFIG_OPEN_CONFIG_FILE_FAILURE if opening the config
 *        file failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_SETFDS_FAILURE if setting file descriptors
 *        failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_EXEC_PRIVATE_FAILURE if executing the
 *        private command failed.
 *      - AGENTD_ERROR_CONFIG_PRIVSEP_EXEC_SURVIVAL_WEIRDNESS if the process
 *        survived execution (weird!).      
 *      - AGENTD_ERROR_CONFIG_IPC_READ_DATA_FAILURE if reading data from the
 *        config stream failed.
 *      - AGENTD_ERROR_CONFIG_PROC_EXIT_FAILURE if the config proc did not
 *        properly exit.
 *      - AGENTD_ERROR_CONFIG_DEFAULTS_SET_FAILURE if setting the config
 *        defaults failed.
 */
int config_read_proc(
    const struct bootstrap_config* bconf, agent_config_t* conf);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_CONFIG_HEADER_GUARD*/
