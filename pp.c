#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int fd[2];

    pipe(fd);

    int pid = fork();

    if (pid == 0) {
        // 子进程：读
        close(fd[1]);

        char buf[100] = {0};
        read(fd[0], buf, sizeof(buf));

        printf("child read: %s\n", buf);

        close(fd[0]);
    }
    else {
        // 父进程：写
        close(fd[0]);

        char *msg = "hello child";
        write(fd[1], msg, strlen(msg));

        close(fd[1]);
    }

    return 0;
}
