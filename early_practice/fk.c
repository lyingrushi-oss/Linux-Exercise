#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd = open("test.txt", O_RDONLY);

    printf("before fork: fd = %d\n", fd);

    fork();

    char buf[4] = {0};

    read(fd, buf, 3);

    printf("pid=%d, buf=%s\n", getpid(), buf);

    close(fd);

    return 0;
}
