#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static void command_init(command_t *command)
{
    command->argc = 0;
    command->input_file = NULL;
    command->output_file = NULL;
    command->append = 0;
    command->background = 0;

    for (int i = 0; i < MAX_ARGS; i++)
        command->argv[i] = NULL;
}

static char *duplicate_string(const char *text)
{
    if (text == NULL)
        return NULL;

    char *copy = malloc(strlen(text) + 1);

    if (copy == NULL)
        return NULL;

    strcpy(copy, text);

    return copy;
}

void parser(const token_list_t *tokens, command_list_t *commands)
{
    commands->count = 0;

    if (tokens == NULL || tokens->count == 0)
        return;

    command_t *current = &commands->commands[0];

    command_init(current);
    commands->count = 1;

    for (int i = 0; i < tokens->count; i++)
    {
        token_t token = tokens->tokens[i];

        /*
         * END token
         */
        if (token.type == TOKEN_END)
        {
            break;
        }

        /*
         * WORD
         *
         * Add the word to argv.
         */
        if (token.type == TOKEN_WORD)
        {
            if (current->argc < MAX_ARGS - 1)
            {
                current->argv[current->argc] =
                    duplicate_string(token.text);

                current->argc++;

                current->argv[current->argc] = NULL;
            }

            continue;
        }

        /*
         * PIPE
         *
         * Start a new command.
         */
        if (token.type == TOKEN_PIPE)
        {
            if (commands->count >= MAX_TOKENS)
                break;

            current = &commands->commands[commands->count];

            command_init(current);
            commands->count++;

            continue;
        }

        /*
         * Input redirection
         *
         * <
         */
        if (token.type == TOKEN_INPUT)
        {
            if (i + 1 < tokens->count &&
                tokens->tokens[i + 1].type == TOKEN_WORD)
            {
                current->input_file =
                    duplicate_string(tokens->tokens[i + 1].text);

                i++;
            }

            continue;
        }

        /*
         * Output redirection
         *
         * >
         */
        if (token.type == TOKEN_OUTPUT)
        {
            if (i + 1 < tokens->count &&
                tokens->tokens[i + 1].type == TOKEN_WORD)
            {
                current->output_file =
                    duplicate_string(tokens->tokens[i + 1].text);

                current->append = 0;

                i++;
            }

            continue;
        }

        /*
         * Append redirection
         *
         * >>
         */
        if (token.type == TOKEN_APPEND)
        {
            if (i + 1 < tokens->count &&
                tokens->tokens[i + 1].type == TOKEN_WORD)
            {
                current->output_file =
                    duplicate_string(tokens->tokens[i + 1].text);

                current->append = 1;

                i++;
            }

            continue;
        }

        /*
         * Background
         *
         * &
         */
        if (token.type == TOKEN_BACKGROUND)
        {
            current->background = 1;
            continue;
        }
    }
}

void command_list_free(command_list_t *commands)
{
    if (commands == NULL)
        return;

    for (int i = 0; i < commands->count; i++)
    {
        command_t *command = &commands->commands[i];

        for (int j = 0; j < command->argc; j++)
        {
            free(command->argv[j]);
            command->argv[j] = NULL;
        }

        free(command->input_file);
        command->input_file = NULL;

        free(command->output_file);
        command->output_file = NULL;
    }

    commands->count = 0;
}
