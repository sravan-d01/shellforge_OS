#include <stdio.h>
#include <readline/history.h>
#include "history.h"

void print_command_history(void)
{
    HIST_ENTRY **entries = history_list();

    printf("------ Command History ------\n");

    if (entries != NULL)
    {
        for (int i = 0; entries[i] != NULL; i++)
        {
            printf("%d  %s\n",
                   history_base + i,
                   entries[i]->line);
        }
    }
}
