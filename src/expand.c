#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "expand.h"

static char *expand_variable(const char *text)
{
    if (text == NULL)
        return NULL;

    /*
     * If there is no '$', return a copy of the
     * original string.
     */
    if (strchr(text, '$') == NULL)
    {
        char *copy = malloc(strlen(text) + 1);

        if (copy == NULL)
            return NULL;

        strcpy(copy, text);
        return copy;
    }

    char result[MAX_TOKEN_LEN];
    int r = 0;

    for (int i = 0; text[i] != '\0' && r < MAX_TOKEN_LEN - 1; i++)
    {
        if (text[i] != '$')
        {
            result[r++] = text[i];
            continue;
        }

        /*
         * Handle $HOME, $PATH, $USER, etc.
         */
        i++;

        if (text[i] == '\0')
            break;

        char variable[128];
        int v = 0;

        while (text[i] != '\0' &&
               ((text[i] >= 'A' && text[i] <= 'Z') ||
                (text[i] >= 'a' && text[i] <= 'z') ||
                (text[i] >= '0' && text[i] <= '9') ||
                text[i] == '_'))
        {
            if (v < (int)sizeof(variable) - 1)
                variable[v++] = text[i];

            i++;
        }

        variable[v] = '\0';

        /*
         * We moved one character too far.
         * Move back so the outer loop processes it.
         */
        i--;

        const char *value = getenv(variable);

        if (value != NULL)
        {
            for (int j = 0;
                 value[j] != '\0' && r < MAX_TOKEN_LEN - 1;
                 j++)
            {
                result[r++] = value[j];
            }
        }
    }

    result[r] = '\0';

    char *expanded = malloc(strlen(result) + 1);

    if (expanded == NULL)
        return NULL;

    strcpy(expanded, result);

    return expanded;
}

static void expand_command(command_t *command)
{
    if (command == NULL)
        return;

    /*
     * Expand command arguments.
     */
    for (int i = 0; i < command->argc; i++)
    {
        char *expanded = expand_variable(command->argv[i]);

        if (expanded != NULL)
        {
            free(command->argv[i]);
            command->argv[i] = expanded;
        }
    }

    /*
     * Expand input redirection filename.
     */
    if (command->input_file != NULL)
    {
        char *expanded = expand_variable(command->input_file);

        if (expanded != NULL)
        {
            free(command->input_file);
            command->input_file = expanded;
        }
    }

    /*
     * Expand output redirection filename.
     */
    if (command->output_file != NULL)
    {
        char *expanded = expand_variable(command->output_file);

        if (expanded != NULL)
        {
            free(command->output_file);
            command->output_file = expanded;
        }
    }
}

void expand_commands(command_list_t *commands)
{
    if (commands == NULL)
        return;

    for (int i = 0; i < commands->count; i++)
    {
        expand_command(&commands->commands[i]);
    }
}
