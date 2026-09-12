#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;

    printf("before fork\n");

    pid = fork();

    if (pid == 0)
    {
        printf("I am child, PID = %d\n", getpid());
	sleep(20);
	printf("child: new ppid is %d\n",getppid());	

    }
    else
    {
        printf("I am parent, PID = %d\n", getpid());
	sleep(3); 
	printf("parent exsists\n");
    }

    return 0;
}
