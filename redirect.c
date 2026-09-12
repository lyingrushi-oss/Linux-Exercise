#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int main()
{
	int fd;
	fd = open("output2.txt",O_WRONLY|O_CREAT|O_TRUNC,0644);
	if(fd==0)
	{
	perror("open");
        
	return 1;

	}
	dup2(fd,STDOUT_FILENO);
	close (fd);
	printf("hello from redirect!!\n");
	return 0;
}
