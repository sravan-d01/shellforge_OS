#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    char buffer[100];

    int fd = open("mypipe", O_RDONLY);

    read(fd, buffer, sizeof(buffer));

    printf("Received: %s\n", buffer);

    close(fd);

    return 0;
}
