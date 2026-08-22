#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "executor.h"

static int setup_input_redirection(command_t *command)
{
    if (command->input_file == NULL)
        return 0;

    int fd = open(command->input_file, O_RDONLY);

    if (fd < 0)
    {
        perror(command->input_file);
        return -1;
    }

    if (dup2(fd, STDIN_FILENO) < 0)
    {
        perror("dup2");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

static int setup_output_redirection(command_t *command)
{
    if (command->output_file == NULL)
        return 0;

    int flags = O_WRONLY | O_CREAT;

    if (command->append)
        flags |= O_APPEND;
    else
        flags |= O_TRUNC;

    int fd = open(command->output_file, flags, 0644);

    if (fd < 0)
    {
        perror(command->output_file);
        return -1;
    }

    if (dup2(fd, STDOUT_FILENO) < 0)
    {
        perror("dup2");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

void execute_commands(command_list_t *commands)
{
    if (commands == NULL || commands->count == 0)
        return;

    int previous_pipe_read = -1;
    pid_t pids[MAX_TOKENS];
    int pid_count = 0;

    for (int i = 0; i < commands->count; i++)
    {
        command_t *command = &commands->commands[i];

        if (command->argc == 0)
            continue;

        int pipe_fd[2] = {-1, -1};

        /*
         * Create a pipe if this is not the last command.
         */
        if (i < commands->count - 1)
        {
            if (pipe(pipe_fd) < 0)
            {
                perror("pipe");

                if (previous_pipe_read != -1)
                    close(previous_pipe_read);

                return;
            }
        }

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");

            if (pipe_fd[0] != -1)
                close(pipe_fd[0]);

            if (pipe_fd[1] != -1)
                close(pipe_fd[1]);

            if (previous_pipe_read != -1)
                close(previous_pipe_read);

            return;
        }

        /*
         * Child process
         */
        if (pid == 0)
        {
            /*
             * Input from previous command in a pipeline.
             */
            if (previous_pipe_read != -1)
            {
                if (dup2(previous_pipe_read, STDIN_FILENO) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }

                close(previous_pipe_read);
            }

            /*
             * Output to next command in a pipeline.
             */
            if (pipe_fd[1] != -1)
            {
                if (dup2(pipe_fd[1], STDOUT_FILENO) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }

                close(pipe_fd[1]);
            }

            if (pipe_fd[0] != -1)
                close(pipe_fd[0]);

            /*
             * Explicit input redirection.
             */
            if (setup_input_redirection(command) < 0)
                exit(EXIT_FAILURE);

            /*
             * Explicit output redirection.
             */
            if (setup_output_redirection(command) < 0)
                exit(EXIT_FAILURE);

            /*
             * Execute external command.
             */
            execvp(command->argv[0], command->argv);

            /*
             * execvp() only returns when execution fails.
             */
            perror(command->argv[0]);
            exit(EXIT_FAILURE);
        }

        /*
         * Parent process
         */
        pids[pid_count++] = pid;

        if (previous_pipe_read != -1)
            close(previous_pipe_read);

        if (pipe_fd[1] != -1)
            close(pipe_fd[1]);

        previous_pipe_read = pipe_fd[0];
    }

    /*
     * Close the remaining pipe read end.
     */
    if (previous_pipe_read != -1)
        close(previous_pipe_read);

    /*
     * Wait for foreground commands.
     *
     * If the last command is running in background,
     * don't wait for the pipeline.
     */
    if (commands->commands[commands->count - 1].background)
        return;

    for (int i = 0; i < pid_count; i++)
    {
        waitpid(pids[i], NULL, 0);
    }
}
