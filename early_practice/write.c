#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
int main()
{
	int fd= open("test.txt",O_RDWR,0644);
	if(fd==-1)
	{
	perror("open");
	return 1;
	}
	char buf[6]={0};
	ssize_t ret0 = read(fd, buf, 5);

    printf("read ret = %ld\n", ret0);
	ssize_t ret= write(fd,buf,3);
	printf("write ret= %ld\n",ret);
	close(fd);
	return 0;
}
