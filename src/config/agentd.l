/**
 * \file config/agentd.l
 *
 * \brief Lexical scanner for agentd configuration.
 *
 * \copyright 2018 Velo Payments, Inc.  All rights reserved.
 */

%{
#include <agentd/config.h>
#include <config/agentd.tab.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

FILE* config_set_input_filedescriptor(
    yyscan_t scanner, int fd, void* state);
%}

%option reentrant
%option bison-bridge
%option noyywrap nounput noinput

%%

: {
    /* colon token */
    yylval->string = ":";
    return COLON;
}

([0-9]){1,3}[.]([0-9]){1,3}[.]([0-9]){1,3}[.]([0-9]){1,3} {
    /* IP Address */
    yylval->addr = (struct in_addr*)malloc(sizeof(struct in_addr));
    if (1 != inet_pton(AF_INET, yytext, yylval->addr))
    {
        free(yylval->addr);
        yylval->string = strdup(yytext);

        return INVALID_IP;
    }

    return IP;
}

([0-9])+ {
    /* a number token */
    yylval->number = atol(yytext);
    return NUMBER;
}

logdir {
    /* logdir keyword */
    yylval->string = "logdir";
    return LOGDIR;
}

loglevel {
    /* loglevel keyword */
    yylval->string = "loglevel";
    return LOGLEVEL;
}

secret {
    /* secret keyword */
    yylval->string = "secret";
    return SECRET;
}

rootblock {
    /* rootblock keyword */
    yylval->string = "rootblock";
    return ROOTBLOCK;
}

datastore {
    /* datastore keyword */
    yylval->string = "datastore";
    return DATASTORE;
}

listen {
    /* listen keyword */
    yylval->string = "listen";
    return LISTEN;
}

chroot {
    /* chroot keyword */
    yylval->string = "chroot";
    return CHROOT;
}

usergroup {
    /* usergroup keyword */
    yylval->string = "usergroup";
    return USERGROUP;
}

(([^./[:space:]][^/[:space:]]*)|[.])([/][^./[:space:]][^/[:space:]]*)+ {
    /* path token */
    yylval->string = strdup(yytext);
    return PATH;
}

[A-Za-z_][A-Za-z0-9_]* {
    /* identifier token */
    yylval->string = strdup(yytext);
    return IDENTIFIER;
}

[[:space:]] /* ignore whitespace */

. {
    /* invalid token */
    yylval->string = strdup(yytext);
    return INVALID;
}

%%

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
    yyscan_t scanner, int fd, void* st)
{
    YY_BUFFER_STATE* state = (YY_BUFFER_STATE*)st;

    FILE* f = fdopen(fd, "r");

    if (NULL != f)
    {
        *state = yy_create_buffer(f, YY_BUF_SIZE, scanner);
        yy_switch_to_buffer(*state, scanner);
    }

    return f;
}
