/*
TCP服务器：创建socket,准备地址，绑定IP，监听

*/
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000

int main(void)
{

    //#1.创建TCP socket
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd==-1)
    {
        //创建失败
        perror("socket");
        return 1;
    }
    //#2.准备服务器本地地址
    struct sockaddr_in server_addr={0};
    server_addr.sin_family=AF_INET;//IPv4协议族
    server_addr.sin_port=htons(SERVER_PORT);
    int ret=inet_pton(AF_INET,SERVER_IP,&server_addr.sin_addr);
    if(ret!=1)
    {
        //转换失败
        if(ret==0)
        {
            printf("invalid IP\n");

        }
        else
        {
            perror("inet_pton");
        }
       
        return 1;
    }
    //#3.把socket和本地IP、端口绑定
    ret=bind(listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr));
    if(ret==-1)
    {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    //#4.进入监听状态
    ret=listen(listen_fd,5);//参数5：accept队列长度
    if(ret==-1)
    {
        perror("listen");
        close(listen_fd);
        return 1;
    }
    
    printf("listening on ip:%s,port:%d...\n",SERVER_IP,SERVER_PORT);

    //#5.accept
    struct sockaddr_in client_addr;
    socklen_t client_len=sizeof(client_addr);
    int client_fd;
    while(1)
    {
        printf("waiting for client...\n");
        client_fd=accept(listen_fd,(struct sockaddr*)&client_addr,&client_len);
        if(ret==-1)
        {
            perror("accept");
            close(listen_fd);
            break;
        }
        char ip[64];
        //转换成字符串IP
        const char* result=inet_ntop(AF_INET,&client_addr.sin_addr,ip,client_len);
        if(result==NULL)
        {
            perror("inet_ntop");
            //close(listen_fd);
            close(client_fd);
            continue;
        }
        //转换端口号
        uint16_t port=ntohs(client_addr.sin_port);
        printf("accept client:IP:%s,port:%d\n",ip,port);
        
        //#6.读写操作
        ssize_t n;
        char buf[64];
        n= read(client_fd,buf,sizeof(buf)-1);
        if(n==-1)
        {
            perror("read");
            //close(listen_fd);
            close(client_fd);
            continue;
        }
        else if (n==0)
        {
            /* code */
            printf("client close connect\n");
            close(client_fd);
            continue;
            return 1;
        }
        buf[n]='\0';
        printf("read %zd bytes,%s\n",n,buf);

        char reply[]="hello from server\n";
        n=write(client_fd,reply,sizeof(reply)-1);
        if(n==-1)
        {
            perror("write");
            //close(listen_fd);
            close(client_fd);
            continue;
        }
        printf("write %zd bytes\n",n);
        close(client_fd);
    }

    getchar();
    close(listen_fd);
    //close(client_fd);
    return 0;
}