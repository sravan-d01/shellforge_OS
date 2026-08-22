#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/history.h>
#include <readline/readline.h>

#include "lexer.h"
#include "parser.h"
#include "expand.h"
#include "history.h"

static void print_commands(const command_list_t *commands)
{
    printf("----------- PARSED COMMANDS -----------\n");

    for (int i = 0; i < commands->count; i++)
    {
        const command_t *command = &commands->commands[i];

        printf("Command %d:\n", i);

        printf("  argc      : %d\n", command->argc);

        printf("  argv      :");

        for (int j = 0; j < command->argc; j++)
        {
            printf(" [%s]", command->argv[j]);
        }

        printf("\n");

        if (command->input_file != NULL)
        {
            printf("  input     : %s\n",
                   command->input_file);
        }
        else
        {
            printf("  input     : none\n");
        }

        if (command->output_file != NULL)
        {
            printf("  output    : %s\n",
                   command->output_file);

            if (command->append)
            {
                printf("  mode      : append\n");
            }
            else
            {
                printf("  mode      : overwrite\n");
            }
        }
        else
        {
            printf("  output    : none\n");
        }

        printf("  background: %s\n",
               command->background ? "yes" : "no");
    }

    printf("---------------------------------------\n");
}

int main(void)
{
    printf("=====================================\n");
    printf("         Shellforge\n");
    printf(" A Unix Style Shell written in C\n");
    printf("=====================================\n");

    while (1)
    {
        char *line = readline("shellforge$ ");

        if (line == NULL)
        {
            printf("\nGoodbye!\n");
            break;
        }

        if (strlen(line) == 0)
        {
            free(line);
            continue;
        }

        /*
         * Do not put "exit" into command history.
         */
        if (strcmp(line, "exit") == 0)
        {
            free(line);
            printf("Exiting...\n");
            break;
        }

        /*
         * Do not put "history" itself into history.
         */
        if (strcmp(line, "history") == 0)
        {
            print_command_history();
            free(line);
            continue;
        }

        /*
         * Save normal commands.
         */
        add_history(line);

        /*
         * -----------------------------
         * LEXER
         * -----------------------------
         */
        token_list_t tokens;

        lexer(line, &tokens);

        /*
         * Display tokens.
         */
        token_print(&tokens);

        /*
         * -----------------------------
         * PARSER
         * -----------------------------
         */
        command_list_t commands;

        parser(&tokens, &commands);

        /*
         * -----------------------------
         * EXPAND
         * -----------------------------
         */
        expand_commands(&commands);

        /*
         * Display parsed and expanded commands.
         */
        print_commands(&commands);

        /*
         * Free dynamically allocated
         * command information.
         */
        command_list_free(&commands);

        free(line);
    }

    return 0;
}
