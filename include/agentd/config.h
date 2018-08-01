/**
 * \file agentd/config.h
 *
 * \brief Configuration data structure for agentd.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#ifndef AGENTD_CONFIG_HEADER_GUARD
#define AGENTD_CONFIG_HEADER_GUARD

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

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

#define CONFIG_STREAM_TYPE_BOM 0x00
#define CONFIG_STREAM_TYPE_LOGDIR 0x01
#define CONFIG_STREAM_TYPE_LOGLEVEL 0x02
#define CONFIG_STREAM_TYPE_SECRET 0x03
#define CONFIG_STREAM_TYPE_ROOTBLOCK 0x04
#define CONFIG_STREAM_TYPE_DATASTORE 0x05
#define CONFIG_STREAM_TYPE_LISTEN_ADDR 0x06
#define CONFIG_STREAM_TYPE_CHROOT 0x07
#define CONFIG_STREAM_TYPE_USERGROUP 0x08
#define CONFIG_STREAM_TYPE_EOM 0x80
#define CONFIG_STREAM_TYPE_ERROR 0xFF

/**
 * \brief Root of the agent configuration AST.
 */
typedef struct agent_config
{
    disposable_t hdr;
    const char* logdir;
    bool loglevel_set;
    int64_t loglevel;
    const char* secret;
    const char* rootblock;
    const char* datastore;
    config_listen_address_t* listen_head;
    const char* chroot;
    config_user_group_t* usergroup;
} agent_config_t;

/**
 * \brief Union for parser.
 */
typedef union config_val
{
    int64_t number;
    const char* string;
    struct in_addr* addr;
    agent_config_t* config;
    config_user_group_t* usergroup;
    config_listen_address_t* listenaddr;
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
 * \returns 0 on success and non-zero on failure.
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
 * \returns 0 on success and non-zero on failure.
 */
int config_read_block(int s, agent_config_t* conf);

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
 * \returns 0 on success and non-zero on failure.
 */
int config_read_proc(struct bootstrap_config* bconf, agent_config_t* conf);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_CONFIG_HEADER_GUARD*/
