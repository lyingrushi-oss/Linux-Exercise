#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000

int main(void)
{
    //#1. 声明变量和缓冲区
    int socket_fd;
    const char msg[]="temp=15.0\n";
    char recv_buf[64];

    //#2.创建socket
    socket_fd=socket(AF_INET,SOCK_STREAM,0);
    if(socket_fd==-1)
    {
        perror("socket");
        return 1;//创建socket失败
    }
    
    //#3.连接
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(SERVER_PORT);
    int ret = inet_pton(AF_INET,SERVER_IP,&addr.sin_addr);
    if(ret!=1)
    {
        if(ret==-1)
        {
            perror("inet_pton");
            
        }
        else 
        {
            //无效IP,IP不存在
            printf("invalid IP\n");
            
        }
        //关闭socket
        close(socket_fd);
        return 1;
    }
    ret=connect(socket_fd,(struct sockaddr*)&addr,sizeof(addr));
    if(ret==-1)
    {
        perror("connect");
        close(socket_fd);
        return 1;
    }
    //连接成功
    printf("connect success,IP=%s,port=%d\n",SERVER_IP,SERVER_PORT);
    //向服务器写数据
    ssize_t count;
    count=write(socket_fd,msg,sizeof(msg)-1);
    if(count<0)
    {
        perror("write");
        close(socket_fd);
        return 1;
    }
    printf("write %zd/%zu bytes\n",count,(sizeof(msg)-1));
    //读取
    count=read(socket_fd,recv_buf,sizeof(recv_buf)-1);
    if(count==0)
    {
        printf("server close connect\n");
        return 1;
    }
    else if(count<0)
    {
        perror("write");
        return 1;
    }
    else
    {
        recv_buf[count]='\0';
        printf("read:%s,%zd bytes\n",recv_buf,count);
    }

    return 0;
}