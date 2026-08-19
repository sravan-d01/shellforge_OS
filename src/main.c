#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "lexer.h"
#include "history.h"

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

        /* Save normal commands */
        add_history(line);

        /* Tokenize the command */
        token_list_t list;

        lexer(line, &list);

        /* Display tokens */
        token_print(&list);

        free(line);
    }

    return 0;
}
