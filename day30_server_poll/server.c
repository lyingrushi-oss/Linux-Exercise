#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

static volatile sig_atomic_t stop_requested;

static void handle_stop_signal(int signo)
{
    //(void)signo;
    stop_requested=signo;
}

/*宏定义*/
#define SERVER_IP "127.0.0.1" //服务器IP地址
#define SERVER_PORT 9000    //服务器端口
#define MAX_CLIENTS 4       //连接的客户端数量

#define SLOT_LISTEN 0 //监听fd下标
#define SLOT_CLIENT_BEGIN 1  //客户端fd起始下标
#define SLOT_COUNT (1+MAX_CLIENTS) //槽位个数

int main(void)
{

    //安装信号处理函数
    struct sigaction sa={0};
    sa.sa_handler=handle_stop_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;

    //绑定信号处理函数
    if(sigaction(SIGINT,&sa,NULL)==-1)//interrupt:终端按Ctrl C
    {
        perror("sigaction SIGINT");
        return 1;
    }

    if(sigaction(SIGTERM,&sa,NULL)==-1)//terminate:请求程序正常终止
    {
        perror("sigaction SITERM");
        return 1;
    }


    //开启服务器
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd==-1)
    {

        perror("socket");
        return 1;
    }

    //bind之前寻址
    struct sockaddr_in addr={
        .sin_family=AF_INET,    //IPv4协议族
        .sin_port=htons(SERVER_PORT),//host short(主机字节序) to 网络字节序network short
       
    };

    //将IP地址由presentation转换成net形式写入结构体
    if(inet_pton(AF_INET,SERVER_IP,&addr.sin_addr)!=1)
    {
        fprintf(stderr,"invalid server IP\n");
        close(listen_fd);
        return 1;
    }

    //😅
    int reuse = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
                &reuse, sizeof(reuse)) == -1) {
        perror("setsockopt");
        close(listen_fd);
        return 1;
    }

    //bind接口通过长度和协议族类型还有listen_fd socket类型判断IPv4/IPv6
    //bind接口兼容IPv4/IPv6类型的协议族：所以先选择sockaddr_in/in6,随后强制类型转换兼容接口，传入长度
    int ret=bind(listen_fd,(struct sockaddr *)&addr,(socklen_t)sizeof(addr));
    if(ret==-1)
    {
        perror("bind");
        close(listen_fd);
        return 1;
    }
    //开始监听
    //第二个参数：内核中等待accept的已建立TCP连接的队列长度提示
    ret=listen(listen_fd,MAX_CLIENTS);// 内核中等待 accept 的已完成连接队列长度提示
    if(ret==-1)
    {
        perror("listen");
        close(listen_fd);
        return 1;
    }



    struct pollfd fds[SLOT_COUNT];
    for(int i=0;i<SLOT_COUNT;i++)
    {
        //遍历初始化数组
        fds[i].fd=-1;
        fds[i].events=0;
        fds[i].revents=0;
    }

    //配置监听fd
    fds[SLOT_LISTEN].fd=listen_fd;
    fds[SLOT_LISTEN].events=POLLIN;
    printf("[server] listening on :%s : %d\n"
            ,SERVER_IP,SERVER_PORT);
     
    //等待连接
    while(stop_requested==0)
    {
        int ready=poll(fds,SLOT_COUNT,-1);//永久等待
        if(ready==-1)
        {
            if(errno==EINTR)
            {
                continue;
            }
            perror("poll");
            break;
        }



        //🌙处理监听fd，处理新连接：有新的客户端连接
        if(fds[SLOT_LISTEN].revents&POLLIN)
        {
            //建立新的客户端连接
            struct sockaddr_in client_addr;
            socklen_t client_len=sizeof(client_addr);
            int client_fd=accept(fds[SLOT_LISTEN].fd,
                                (struct sockaddr*)&client_addr,&client_len);
            if(client_fd==-1)
            {
                perror("accept");//accept失败，跳出此次循环等待其他连接
                continue;
            }

            //成功获取连接客户端fd
            int free_slot=-1;

            for(int i=SLOT_CLIENT_BEGIN;
                i<SLOT_COUNT;i++)
            {
                if(fds[i].fd!=-1)
                {
                    continue;
                }
                //找到空闲槽位
                free_slot=i;
                break;
            }

            if(free_slot==-1)
            {
                printf("槽位已满，无空闲槽位,fd=%d\n",client_fd);
                close(client_fd);//关闭新接受的客户端fd
                continue;
            }

            //写入空闲槽位
            fds[free_slot].fd=client_fd;
            fds[free_slot].events=POLLIN;
            fds[free_slot].revents=0;

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET,&client_addr.sin_addr,client_ip,sizeof(client_ip));
            printf("[server] client connected:%s:%d,free_slot=%d,fd=%d\n",client_ip,
                ntohs(client_addr.sin_port),free_slot,fds[free_slot].fd);


        }


        //🌟遍历处理客户端通信
        /*
        n > 0   // 缓冲区还有数据，继续处理
        n == 0  // 数据已经读完，EOF，关闭连接
        n == -1 // 读取错误
        */
        for(int i=SLOT_CLIENT_BEGIN;i<SLOT_COUNT;i++)
        {
            //无效fd
            if(fds[i].fd==-1) continue;
            if(fds[i].revents&(POLLIN|POLLHUP))
            {
                //打印revent
                printf("[debug] slot=%d,fd=%d,revents=0x%x\n",
                i,fds[i].fd,fds[i].revents);
                //有数据待读取
                char buf[128];
                //读取数据
                ssize_t n= read(fds[i].fd,buf,sizeof(buf)-1);
                if(n>0)
                {
                    //1️⃣成功接收到数据
                    buf[n]='\0';
                    printf("[server] slot=%d,fd=%d,recv=%zd bytes,%s\n",
                    i,fds[i].fd,n,buf);
                }
                else if(n==0)
                {
                    //2️⃣EOF：缓冲区没有数据且写端已经关闭
                    printf("[server] client disconnect: slot=%d,fd=%d\n",
                    i,fds[i].fd);
                    //回收资源
                    close(fds[i].fd);
                    fds[i].fd=-1;
                    fds[i].events=0;
                    fds[i].revents=0;
                }
                else
                {
                    //3️⃣出错：返回-1
                    perror("read");
                    close(fds[i].fd);
                    fds[i].fd=-1;
                    fds[i].events=0;
                    fds[i].revents=0;
                }
            }

            //🌟错误处理:fd有效且发生错误
            if(fds[i].fd!=-1&&(fds[i].revents&(POLLNVAL|POLLERR)))
            {
                //发生错误，回收资源
                printf("[server] poll error :slot=%d,fd=%d,revents=0x%x\n",
                i,fds[i].fd,fds[i].revents);
                close(fds[i].fd);
                fds[i].fd=-1;
                fds[i].events=0;
                fds[i].revents=0;
            }
        }

        
        

        #if 0
        //🌟处理客户端挂起:先处理读取，再处理挂起，可以避免缓冲区还有数据未读取
        for(int i=SLOT_CLIENT_BEGIN;i<SLOT_COUNT;i++)
        {
            if(fds[i].revents&POLLHUP)
            {
                printf("[server] client slot=%d,fd=%d hang up\n",
                i,fds[i].fd);
                close(fds[i].fd);
                fds[i].fd=-1;
            }
        }
        #endif
        
    }

    printf("\n[server] stop requested, cleaning up...\n");

    //清理资源
    for(int i=SLOT_CLIENT_BEGIN;i<SLOT_COUNT;i++)
    {
        if(fds[i].fd!=-1)
        {
            close(fds[i].fd);
            fds[i].fd=-1;
        }
    }


    close(listen_fd);
    printf("[server] shutdown complete\n");
    if(stop_requested==SIGINT)
    {
        printf("\n[exit] by SIGINT\n");
    }
    else if(stop_requested==SIGTERM)
    {
        printf("\n[exit] by SIGTERM\n");
    }
    return 0;
}