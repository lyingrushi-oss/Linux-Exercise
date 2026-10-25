#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>

int main(void)
{
    int fd[2];
    int ret=pipe(fd);
    if(ret==-1)
    {
        perror("pipe");
        return 1;
    }

    int read_fd=fd[0];
    int write_fd=fd[1];

    //读取当前read_fd的状态
    int flag=fcntl(read_fd,F_GETFL);

    //获取失败
    if(flag==-1)
    {
        perror("fcntl GET_FL");
        return 1;
    }

    //设置非阻塞读取
    if(fcntl(read_fd,F_SETFL,flag|O_NONBLOCK)==-1)
    {
        perror("set read flag");
        return 1;
    }

    char buf[32];
    errno=0;
   

    struct pollfd pfd={
        .fd=read_fd,
        .events=POLLIN,
        .revents=0,
    };
    int ready=poll(&pfd,1,500);//返回值为就绪fd个数，超时时间是500ms

    printf("poll empty: ready=%d,revents=0x%x\n",
    ready,pfd.revents);

    ssize_t n=read(read_fd,buf,sizeof(buf)-1);
    //打印读取结果
    printf("first read: n=%td,errno=%d(%s)\n",n,errno,strerror(errno));

    //写
    const char msg[]="25.6";

    if(write(write_fd,msg,sizeof(msg)-1)==-1)
    {
        perror("write");
        return 1;
    }
    close(write_fd);
 
    //读取
    ready=poll(&pfd,1,500);
    printf("poll data: ready=%d,POLLIN=%d,revents=0x%x\n",
            ready,(pfd.revents&POLLIN)!=0,pfd.revents);
    errno=0;
    n=read(read_fd,buf,sizeof(buf)-1);
    if(n>0)
    {
        buf[n]='\0';
        printf("second read: n=%td,data=%s\n",n,buf);
    }

    

    ready=poll(&pfd,1,500);
    printf("poll closed: ready=%d,POLLHUP=%d,revents=0x%x\n",
            ready,(POLLHUP&pfd.revents)!=0,pfd.revents);
    errno=0;
    n=read(read_fd,buf,sizeof(buf)-1);
    printf("third read: n=%td,errno=%d(%s)\n",
        n,errno,strerror(errno));

    return 0;
}