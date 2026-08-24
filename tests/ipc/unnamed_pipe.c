#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(void)
{
    int fd[2];

    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return 1;
    }

    if (pid > 0)
    {
        printf("Parent PID: %d\n", getpid());
        printf("Child PID : %d\n", pid);

        close(fd[0]);

        char msg[] = "Hello Child";

        write(fd[1], msg, strlen(msg) + 1);

        close(fd[1]);

        wait(NULL);
    }
    else
    {
        close(fd[1]);

        char buffer[100];

        sleep(30);

        read(fd[0], buffer, sizeof(buffer));

        printf("Child Received: %s\n", buffer);

        close(fd[0]);
    }

    return 0;
}
