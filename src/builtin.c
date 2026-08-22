#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "builtin.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/*
 * Shellforge keeps track of the previous working directory.
 *
 * This allows:
 *
 *     cd -
 *
 * to return to the previous location.
 */
static char previous_dir[PATH_MAX] = "";
static int shell_exit_requested = 0;


/* ---------------------------------------------------------
 * Utility
 * --------------------------------------------------------- */

static void print_builtin_header(const char *name)
{
    printf("\n");
    printf("[BUILTIN] %s\n", name);
}

static int current_directory(char *buffer, size_t size)
{
    if (getcwd(buffer, size) == NULL)
    {
        perror("[ERROR] getcwd");
        return 0;
    }

    return 1;
}


/* ---------------------------------------------------------
 * PWD
 * --------------------------------------------------------- */

static builtin_result_t builtin_pwd(command_t *command)
{
    (void)command;

    char cwd[PATH_MAX];

    print_builtin_header("pwd");

    if (!current_directory(cwd, sizeof(cwd)))
        return BUILTIN_FAILED;

    printf("[PATH]   %s\n", cwd);
    printf("[STATUS] OK\n");

    return BUILTIN_DONE;
}


/* ---------------------------------------------------------
 * CD
 * --------------------------------------------------------- */

static builtin_result_t builtin_cd(command_t *command)
{
    char old_directory[PATH_MAX];
    char new_directory[PATH_MAX];

    if (!current_directory(old_directory, sizeof(old_directory)))
        return BUILTIN_FAILED;

    /*
     * No argument:
     *
     * cd
     *
     * moves to HOME.
     */
    if (command->argc == 1)
    {
        const char *home = getenv("HOME");

        if (home == NULL || home[0] == '\0')
        {
            fprintf(stderr,
                    "[SHELLFORGE] HOME is not defined\n");

            return BUILTIN_FAILED;
        }

        if (chdir(home) != 0)
        {
            fprintf(stderr,
                    "[SHELLFORGE] cd: %s: %s\n",
                    home,
                    strerror(errno));

            return BUILTIN_FAILED;
        }
    }

    /*
     * cd -
     *
     * Switch to the previous directory.
     */
    else if (strcmp(command->argv[1], "-") == 0)
    {
        if (previous_dir[0] == '\0')
        {
            fprintf(stderr,
                    "[SHELLFORGE] cd: previous directory unavailable\n");

            return BUILTIN_FAILED;
        }

        if (chdir(previous_dir) != 0)
        {
            fprintf(stderr,
                    "[SHELLFORGE] cd: %s: %s\n",
                    previous_dir,
                    strerror(errno));

            return BUILTIN_FAILED;
        }
    }

    /*
     * Normal:
     *
     * cd <directory>
     */
    else
    {
        if (chdir(command->argv[1]) != 0)
        {
            fprintf(stderr,
                    "[SHELLFORGE] cd: %s: %s\n",
                    command->argv[1],
                    strerror(errno));

            return BUILTIN_FAILED;
        }
    }

    /*
     * Find the directory after chdir().
     */
    if (!current_directory(new_directory, sizeof(new_directory)))
        return BUILTIN_FAILED;

    /*
     * Save the directory we came from.
     */
    strncpy(previous_dir,
            old_directory,
            sizeof(previous_dir) - 1);

    previous_dir[sizeof(previous_dir) - 1] = '\0';

    print_builtin_header("cd");

    printf("[FROM]   %s\n", old_directory);
    printf("[TO]     %s\n", new_directory);
    printf("[STATUS] DIRECTORY CHANGED\n");

    /*
     * For `cd -`, print the new location like normal shells.
     */
    if (command->argc >= 2 &&
        strcmp(command->argv[1], "-") == 0)
    {
        printf("[PWD]    %s\n", new_directory);
    }

    return BUILTIN_DONE;
}


/* ---------------------------------------------------------
 * HELP
 * --------------------------------------------------------- */

static builtin_result_t builtin_help(command_t *command)
{
    (void)command;

    print_builtin_header("help");

    printf("\n");
    printf("Shellforge builtin commands\n");
    printf("--------------------------------\n");
    printf("  pwd       Show current directory\n");
    printf("  cd        Change directory\n");
    printf("  cd <dir>  Change to specified directory\n");
    printf("  cd -      Return to previous directory\n");
    printf("  exit      Leave Shellforge\n");
    printf("  help      Show this message\n");
    printf("--------------------------------\n");
    printf("\n");

    return BUILTIN_DONE;
}


/* ---------------------------------------------------------
 * EXIT
 * --------------------------------------------------------- */

static builtin_result_t builtin_exit(command_t *command)
{
    (void)command;

    print_builtin_header("exit");

    printf("[STATUS] SHUTTING DOWN SHELLFORGE\n");

    shell_exit_requested = 1;

    return BUILTIN_EXIT;
}


/* ---------------------------------------------------------
 * Builtin dispatch table
 * --------------------------------------------------------- */

typedef builtin_result_t (*builtin_function_t)(command_t *);

typedef struct
{
    const char *name;
    builtin_function_t function;
} builtin_entry_t;


static const builtin_entry_t builtin_table[] =
{
    { "pwd",  builtin_pwd  },
    { "cd",   builtin_cd   },
    { "help", builtin_help  },
    { "exit", builtin_exit  }
};


static const size_t builtin_count =
    sizeof(builtin_table) / sizeof(builtin_table[0]);


/* ---------------------------------------------------------
 * Public dispatcher
 * --------------------------------------------------------- */

builtin_result_t builtin_execute(command_t *command)
{
    if (command == NULL ||
        command->argc == 0 ||
        command->argv[0] == NULL)
    {
        return BUILTIN_NOT_FOUND;
    }

    for (size_t i = 0; i < builtin_count; i++)
    {
        if (strcmp(command->argv[0],
                   builtin_table[i].name) == 0)
        {
            return builtin_table[i].function(command);
        }
    }

    /*
     * Not a builtin.
     *
     * The execution layer in a later milestone can
     * handle normal external commands.
     */
    return BUILTIN_NOT_FOUND;
}


const char *builtin_previous_directory(void)
{
    if (shell_exit_requested)
        return NULL;

    return previous_dir;
}
