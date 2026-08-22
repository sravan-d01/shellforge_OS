#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "lexer.h"
#include "parser.h"
#include "expand.h"
#include "builtin.h"
#include "history.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


/*
 * ---------------------------------------------------------
 * Shellforge string duplication
 * ---------------------------------------------------------
 *
 * We use our own function instead of strdup() so that the
 * program compiles cleanly with -std=c11.
 */
static char *forge_strdup(const char *text)
{
    size_t length;
    char *copy;

    if (text == NULL)
        return NULL;

    length = strlen(text) + 1;

    copy = malloc(length);

    if (copy == NULL)
        return NULL;

    memcpy(copy, text, length);

    return copy;
}


/*
 * ---------------------------------------------------------
 * Shellforge prompt
 * ---------------------------------------------------------
 *
 * The prompt displays the current working directory.
 *
 * Example:
 *
 *     forge@/root $
 *     forge@/tmp $
 *
 */
static char *shellforge_prompt(void)
{
    char cwd[PATH_MAX];
    char prompt[PATH_MAX + 32];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        return forge_strdup("forge@? $ ");
    }

    snprintf(
        prompt,
        sizeof(prompt),
        "forge@%s $ ",
        cwd
    );

    return forge_strdup(prompt);
}


/*
 * ---------------------------------------------------------
 * Shellforge banner
 * ---------------------------------------------------------
 */
static void shellforge_banner(void)
{
    printf("\n");
    printf("╔══════════════════════════════════════╗\n");
    printf("║            SHELLFORGE 3.1            ║\n");
    printf("║       Interactive Builtin Core        ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("\n");
}


/*
 * ---------------------------------------------------------
 * Main Shell Loop
 * ---------------------------------------------------------
 */
int main(void)
{
    shellforge_banner();

    while (1)
    {
        char *prompt;
        char *line;

        /*
         * Build the prompt using the current directory.
         */
        prompt = shellforge_prompt();

        if (prompt == NULL)
        {
            fprintf(
                stderr,
                "[SHELLFORGE] unable to create prompt\n"
            );

            return EXIT_FAILURE;
        }

        /*
         * Read command from the user.
         */
        line = readline(prompt);

        free(prompt);

        /*
         * Ctrl+D / EOF
         */
        if (line == NULL)
        {
            printf("\n");
            printf("[SHELLFORGE] input stream closed\n");
            break;
        }

        /*
         * Ignore empty input.
         */
        if (line[0] == '\0')
        {
            free(line);
            continue;
        }

        /*
         * Store command in readline history.
         */
        add_history(line);


        /*
         * =================================================
         * LEXER
         * =================================================
         */
        token_list_t tokens;

        lexer(line, &tokens);


        /*
         * =================================================
         * PARSER
         * =================================================
         */
        command_list_t commands;

        parser(&tokens, &commands);


        /*
         * =================================================
         * EXPANSION
         * =================================================
         *
         * Example:
         *
         *     echo $HOME
         *
         * becomes:
         *
         *     echo /root
         */
        expand_commands(&commands);


        /*
         * Nothing was produced by the parser.
         */
        if (commands.count == 0)
        {
            command_list_free(&commands);
            free(line);
            continue;
        }


        /*
         * =================================================
         * BUILTIN DISPATCH
         * =================================================
         *
         * Milestone 3.1 builtins:
         *
         *     pwd
         *     cd
         *     help
         *     exit
         *
         */
        for (int i = 0; i < commands.count; i++)
        {
            command_t *command;
            builtin_result_t result;

            command = &commands.commands[i];

            /*
             * Ignore empty command segments.
             */
            if (command->argc == 0)
                continue;

            /*
             * Send command to builtin dispatcher.
             */
            result = builtin_execute(command);


            /*
             * -------------------------------------------------
             * EXIT
             * -------------------------------------------------
             */
            if (result == BUILTIN_EXIT)
            {
                command_list_free(&commands);
                free(line);

                printf("\n");
                printf("[SHELLFORGE] session ended\n");

                return EXIT_SUCCESS;
            }


            /*
             * -------------------------------------------------
             * UNKNOWN COMMAND
             * -------------------------------------------------
             *
             * External command execution belongs to a later
             * milestone.
             */
            if (result == BUILTIN_NOT_FOUND)
            {
                printf("\n");
                printf("[FORGE] external command: %s\n",
                       command->argv[0]);

                printf(
                    "[FORGE] execution engine is reserved "
                    "for a later milestone.\n"
                );
            }
        }


        /*
         * Free parser/expansion memory.
         */
        command_list_free(&commands);

        /*
         * Free readline input.
         */
        free(line);
    }


    /*
     * Normal termination through EOF.
     */
    printf("[SHELLFORGE] goodbye!\n");

    return EXIT_SUCCESS;
}
