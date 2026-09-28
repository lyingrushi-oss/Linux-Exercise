#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdatomic.h>

//定义TCP连接端口号
#define PORT 9000
//定义样本数量
#define SAMPLE_COUNT 10

//定义传感器数据结构体
typedef struct{
    int seq;
    float temp;
}sensor_data_t;

//定义pollfd数组索引枚举
typedef enum{
    FD_SENSOR=0,
    FD_STDIN,
    FD_LISTEN,
    FD_CLIENT,
    FD_COUNT,
}fd_index_t;

static int sensor_pipe[2];
static atomic_bool stop_requested=ATOMIC_VAR_INIT(false);

static int create_listen_socket(uint16_t port);

static void* producer_pthread(void* arg);

int main(void)
{
    //创建sensor_pipe
    int ret=pipe(sensor_pipe);

    //初始化pollfd数组
    //fd=-1的设计：poll会自动忽略负数fd
    struct pollfd fds[FD_COUNT]={
        [FD_SENSOR]={
            .fd=sensor_pipe[0],
            .events=POLLIN,//接收传感器线程的数据
            .revents=0,
        },
        [FD_STDIN]={
            .fd=STDIN_FILENO,
            .events=POLLIN,//接收终端命令
            .revents=0,
        },
        [FD_LISTEN]={
            .fd=-1,
            .events=POLLIN,//有新连接等待accept()
            .revents=0,
        },
        [FD_CLIENT]={
            .fd=-1,
            .events=POLLIN,//当前客户端发来了数据
            .revents=0,
        },
    };

    int failed=0;
    int finished=0;
    int sensor_done=0;
    sensor_data_t latest_data={0};
    int latest_valid=0;

    //进入循环前监听fd
    int listen_fd=create_listen_socket(PORT);
    if(listen_fd==-1)
    {
        return 1;
    }

    //listen_fd创建成功，初始化结构体数组中的fd
    fds[FD_LISTEN].fd=listen_fd;
    printf("[server] listening 127.0.0.1:%d\n",PORT);

    //创建生产者线程
    pthread_t producer;
    ret=pthread_create(&producer,NULL,producer_pthread,NULL);
    if(ret!=0)
    {
        //创建线程失败
        fprintf(stderr,"[producer] pthread create:%s\n",strerror(ret));

        //关闭pipe
        close(sensor_pipe[0]);
        close(sensor_pipe[1]);
        close(listen_fd);

        return 1;//创建失败直接返回，返回之前关闭所有fd
    }

    while(!finished)
    {
        int ready=poll(fds,FD_COUNT,-1);
        if(ready==-1)
        {
            //判断是不是中断导致
            if(ready==EINTR)
            {
                continue;//重新阻塞等待
            }
            //出错
            perror("poll");//打印错误信息
            failed=1;//标记失败
            break;//退出循环
        }

         
        /*
         * 下一步逐块添加：
         * 1. 新客户端连接
         * 2. 传感器数据
         * 3. 终端命令
         * 4. 客户端命令
         * 5. HUP / ERR / NVAL
         */
    
        /*处理新client的连接*/
        if(fds[FD_LISTEN].revents==POLLIN)
        {
            struct sockaddr_in client_addr;
            socklen_t addr_len=sizeof(client_addr);
            int client_fd=accept(fds[FD_LISTEN].fd,(struct sockaddr*)&client_addr,&addr_len);
            if(client_fd==-1)
            {
                perror("accept");
                failed=1;
                finished=1;
                continue;
            }
            if(fds[FD_CLIENT].fd!=-1)
            {
                printf("[server] another client is already connected\n");
                close(client_fd);
            }
            else
            {
                fds[FD_CLIENT].fd=client_fd;
                printf("[server] client connect:%s:%d,fd=%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),client_fd);

            }
        }

        /*传感器数据*/
        if(fds[FD_SENSOR].fd!=-1&&(fds[FD_SENSOR].revents&POLLIN))
        {
            sensor_data_t data;
            ssize_t n=read(fds[FD_SENSOR].fd,&data,sizeof(data));

            if(n==-1)
            {
                perror("sensor read");
                failed=1;
                finished=1;
            }

            else if(n==0)
            {
                printf("sensor read EOF\n");
                sensor_done=1;
            }

            else if(n!=sizeof(data))
            {
                printf("sensor read incomplete bytes:%zd/%zu\n",n,sizeof(data));
                failed=1;
                finished=1;
            }

            else
            {
                //读取成功
                latest_data=data;
                //标记有效
                latest_valid=1;
                printf("sensor read: seq=%d,temperature=%.1f\n",latest_data.seq,latest_data.temp);

                //先转换成文本格式再通过网络把数据发送出去
                if(fds[FD_CLIENT].fd!=-1)
                {
                    char msg[64];
                    int len=snprintf(msg,sizeof(msg),"seq=%d,temperature=%.1f\n",latest_data.seq,latest_data.temp);

                    if(len<0||len>=(int)sizeof(msg))
                    {
                        fprintf(stderr,"[main] format sensor messsage fail\n");

                    }

                    else
                    {
                        ssize_t sent=send(fds[FD_CLIENT].fd,msg,(size_t)len,MSG_NOSIGNAL);

                        if(sent==-1)
                        {
                            perror("[main] send");
                            close(fds[FD_CLIENT].fd);
                            fds[FD_CLIENT].fd=-1;
                        }

                        else if (sent!=len)
                        {
                            /* code */
                            fprintf(stderr,"[main] incomplete send: %zd/%d\n",sent,len);
                        }

                        else
                        {
                            printf("[main] send success\n");
                        }
                        
                    }
                }


            }
        }

        /*终端bash输入*/
        if(fds[FD_STDIN].revents&POLLIN)
        {
            //声明一个缓冲区，接收存放终端输入
            char cmd[64];

            ssize_t n=read(STDIN_FILENO,cmd,sizeof(cmd)-1);

            if(n==-1)
            {
                perror(" stdin read");
                finished=1;
            }

            else if(n==0)
            {
                printf("[main] stdin read EOF\n");
                fds[FD_STDIN].fd=-1;
            }

            else
            {
                cmd[n]='\0';
                cmd[strcspn(cmd,"\r\n")]='\0';

                //把命令按照三种类型处理：status quit unknown

                if(strcmp(cmd,"status")==0)
                {
                    printf("[main] sensor=%s,client=%s,server=%s\n",
                    sensor_done?"stopped":"running",
                    fds[FD_CLIENT].fd>=0?"online":"offline",
                    fds[FD_LISTEN].fd>=0?"listening":"closed"
                    );

                    if(latest_valid)
                    {
                        printf("[status] latest seq=%d,temp=%.1f\n",
                        latest_data.seq,latest_data.temp);
                    }
                    else
                    {
                        printf("[status] latest=none\n");
                    }

                }

                else if(strcmp(cmd,"quit")==0)
                {
                    printf("[main] quit request\n");
                    atomic_store(&stop_requested,true);
                    finished=1;
                }

                else if (cmd[0]!='\0')
                {
                    printf("[main] unknown command:%s\n",cmd);
                }
            }

        }

        if((fds[FD_SENSOR].revents&POLLHUP)&&!(fds[FD_SENSOR].revents&POLLIN))
        {
            //写端关闭且没有数据待读取
            printf("[main] sensor producer closed\n");
            close(fds[FD_SENSOR].fd);
            fds[FD_SENSOR].fd=-1;
            sensor_done=1;
        }

        /*接收读取client数据*/
        if(fds[FD_CLIENT].fd!=-1&&(fds[FD_CLIENT].revents&POLLIN))
        {
            //声明一个接收缓冲区
            char buf[128];

            //接收客户端发来的数据
            ssize_t n=read(fds[FD_CLIENT].fd,buf,sizeof(buf)-1);

            if(n==-1)
            {
                //读取失败
                perror("client read");
                close(fds[FD_CLIENT].fd);
                fds[FD_CLIENT].fd=-1;
            }

            else if(n==0)
            {
                printf("[server] client disconnected\n");
                close(fds[FD_CLIENT].fd);
                fds[FD_CLIENT].fd=-1;
            }

            else
            {
                //读取成功
                buf[n]='\0';
                printf("[server] recv %zd bytes:%s\n",n,buf);
            }
        }
    }

    //无论什么退出原因，都通知并清理生产者线程
    atomic_store(&stop_requested,true);
    int res=pthread_join(producer,NULL);
    if(res!=0)
    {
        fprintf(stderr,"[main] pthread_join fail:%s\n",strerror(res));
        failed=1;
    }

    //关闭仍让有效的传感器读端fd
    if(fds[FD_SENSOR].fd!=-1)
    {
        close(fds[FD_SENSOR].fd);
        fds[FD_SENSOR].fd=-1;
    }

    //循环结束清理网路fd
    if(fds[FD_CLIENT].fd!=-1)
    {
        close(fds[FD_CLIENT].fd);
        fds[FD_CLIENT].fd=-1;
    }
    

    //清理监听socket
    if(fds[FD_LISTEN].fd!=-1)
    {
        close(fds[FD_LISTEN].fd);
        fds[FD_LISTEN].fd=-1;
    }

    printf("[main] clean up and exit\n");
    return failed?1:0;


}

/// @brief 创建监听listen_fd
/// @param port 端口号
/// @return 成功返回fd描述符，失败关闭fd返回-1
static int create_listen_socket(uint16_t port)
{
    int fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd==-1)
    {
        perror("socket");
        return -1;
    }

    int reuse=1;
    if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))==-1)
    {
        perror("setsocketopt");
        close(fd);
        return -1;
    }

    //绑定IP地址和端口
    struct sockaddr_in addr={
        .sin_family=AF_INET,
        .sin_port=htons(port),
        .sin_addr.s_addr=htonl(INADDR_LOOPBACK),
    };

    if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))==-1)
    {
        perror("bind");
        close(fd);
        return -1;
    }

    if(listen(fd,5)==-1)
    {
        perror("listen");
        close(fd);
        return -1;
    }

    return fd;
}

static void* producer_pthread(void* arg)
{
    (void)arg;
    sensor_data_t data={0};
    for(int i=0;i<SAMPLE_COUNT;i++)
    {
        /*先检查一下有没有退出*/
        if(atomic_load(&stop_requested))
        {
            break;
        }
        sleep(2);
        /*sleep以后再检查一次*/
        if(atomic_load(&stop_requested))
        {
            break;
        }
        data.seq=i;
        data.temp=25.0f+i*0.5f;
        

        ssize_t n=write(sensor_pipe[1],&data,sizeof(data));

        if(n==-1)
        {
            perror("producer write");
            break;
        }
        if(n!=sizeof(data))
        {
            fprintf(stderr,"producer incomplete write:%zd/%zu\n",n,sizeof(data));
            break;
        }
        
        
    }
    //生产者任务完成，关闭写端
    close(sensor_pipe[1]);
    printf("[producer] finished\n");
    return NULL;

}