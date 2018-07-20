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

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif  //__cplusplus

#endif /*AGENTD_CONFIG_HEADER_GUARD*/
