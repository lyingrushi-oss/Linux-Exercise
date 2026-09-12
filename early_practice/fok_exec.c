#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid;

    pid = fork();

    if (pid == 0)
    {
        printf("child: PID = %d\n", getpid());

        execl("/bin/ls", "ls", "-l", NULL);

        printf("exec failed\n");
    }
    else
    {
        printf("parent: PID = %d\n", getpid());

        wait(NULL);

        printf("child finished\n");
    }

    return 0;
}
