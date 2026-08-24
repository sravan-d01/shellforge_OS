#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main(void)
{
    mkfifo("mypipe", 0666);

    int fd = open("mypipe", O_WRONLY);

    char msg[] = "Hello from Writer Process";

    write(fd, msg, sizeof(msg));

    close(fd);

    printf("Data sent successfully.\n");

    return 0;
}
