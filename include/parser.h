#ifndef PARSER_H
#define PARSER_H

#include "token.h"

#define MAX_ARGS 64

typedef struct
{
    char *argv[MAX_ARGS];
    int argc;

    char *input_file;
    char *output_file;

    int append;
    int background;
} command_t;

typedef struct
{
    command_t commands[MAX_TOKENS];
    int count;
} command_list_t;

void parser(const token_list_t *tokens, command_list_t *commands);

void command_list_free(command_list_t *commands);

#endif
