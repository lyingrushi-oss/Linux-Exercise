#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main()
{
	int fd[2];
	char buf[100];
	pipe (fd);
	pid_t pid = fork();
	if(pid==0)
	{
	// close(fd[1]);
	 read(fd[0],buf,sizeof(buf));
	 printf("child recv:%s\n",buf); 
	 close(fd[0]);
	}

	else
	{
	 //close (fd[0]);
	 
        char *msg = "hello from parent";

        write(fd[1], msg, strlen(msg) + 1);

        close(fd[1]);
	}
	return 0;
}
