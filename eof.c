#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
int fd[2];
pipe(fd);
int pid = fork();
if(pid==0)
{
//子进程：只读
close(fd[1]);
char buf[20];
int n= read(fd[0],buf,sizeof(buf)-1);
while(n>0)
 {
 buf[n]='\0';

printf("child read:%s\n",buf);
n=read(fd[0],buf,sizeof(buf)-1);
 }
printf("read return:%d\n",n);
}
else
{
// 父进程：只写
        close(fd[0]);

        char *msg = "hello child";

        write(fd[1], msg, strlen(msg));
	printf("parant:第一次写\n");
	sleep(10);
	write(fd[1],msg,strlen(msg));
	printf("parant:第二次写\n");
        close(fd[1]);
}

return 0;
}
