/**
 * \file command/private_command_readconfig.c
 *
 * \brief Read a config file from a stream.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/command.h>
#include <agentd/config.h>
#include <agentd/fds.h>
#include <config/agentd.tab.h>
#include <config/agentd.yy.h>
#include <cbmc/model_assert.h>
#include <stdio.h>
#include <unistd.h>

/**
 * \brief Config context.
 */
typedef struct private_config_context
{
    disposable_t hdr;
    agent_config_t* config;
} private_config_context_t;

/* forward decls. */
static void private_config_context_init(private_config_context_t* ctx);
static void private_config_context_dispose(void* disp);
static void private_config_set_error(
    config_context_t* context, const char* msg);
static void private_config_callback(
    config_context_t* context, agent_config_t* config);

/**
 * \brief Read the config file from a stream.
 */
void private_command_readconfig()
{
    yyscan_t scanner;
    YY_BUFFER_STATE state;
    config_context_t ctx;
    private_config_context_t user_ctx;

    /* set up scanner */
    yylex_init(&scanner);
    private_config_context_init(&user_ctx);
    ctx.set_error = &private_config_set_error;
    ctx.val_callback = &private_config_callback;
    ctx.user_context = &user_ctx;

    /* use AGENTD_FD_CONFIG_IN for the input stream. */
    config_set_input_filedescriptor(scanner, AGENTD_FD_CONFIG_IN, &state);
    if (0 != yyparse(scanner, &ctx))
    {
        private_config_set_error(&ctx, "Parse failure.");
    }

    /* clean up scanner. */
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);

    /* write the config data to the server stream. */
    config_write_block(AGENTD_FD_CONFIG_OUT, user_ctx.config);
}

/**
 * Initialize the private config for this config reader.
 *
 * \param ctx       The private config to init.
 */
static void private_config_context_init(private_config_context_t* ctx)
{
    ctx->hdr.dispose = &private_config_context_dispose;
    ctx->config = NULL;
}

/**
 * Dispose of a private context structure.
 *
 * \param disp      The opaque pointer to this structure.
 */
static void private_config_context_dispose(void* disp)
{
    private_config_context_t* ctx = (private_config_context_t*)disp;

    if (NULL != ctx->config)
    {
        dispose((disposable_t*)ctx->config);
        free(ctx->config);
    }
}

/**
 * \brief Handle an error.
 *
 * \param context       The config context structure.
 * \param msg           The error message.
 */
static void private_config_set_error(
    config_context_t* context, const char* msg)
{
    (void)context;
    char buf[1024];

    int len = snprintf(buf, sizeof(buf), "error: %s\n", msg);

    if (len > 0)
        write(AGENTD_FD_CONFIG_OUT, buf, len);

    exit(1);
}

/**
 * \brief Handle receiving a config structure.
 *
 * \param context       The config context for this callback.
 * \param config        The config structure of which the user takes ownership.
 */
static void private_config_callback(
    config_context_t* context, agent_config_t* config)
{
    private_config_context_t* ctx =
        (private_config_context_t*)context->user_context;

    ctx->config = config;
}
