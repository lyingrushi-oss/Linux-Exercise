#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>

#define SAMPLE_COUNT 5
#define FD_COUNT 2

//定义传感器数据结构体
typedef struct{
    int seq;
    float temp;
}sensor_data_t;

//传感器pipe
static int sensor_pipe[2];

//生产者线程
static void* producer_thread(void * arg)
{
    (void)arg;
    sensor_data_t data[SAMPLE_COUNT];
    for(int i=0;i<SAMPLE_COUNT;i++)
    {
        data[i].seq=i;
        data[i].temp=25.0f+0.5f*i;

        //睡两秒
        sleep(2);

        ssize_t ret=write(sensor_pipe[1],&data[i],sizeof(data[i]));

        if(ret==-1)
        {
            perror("write");
            break;
        }

        if(ret!=sizeof(data[i]))
        {
            fprintf(stderr,"produder write incomplete bytes\n");
            break;
        }

        //成功写入
        printf("[producer] write data:seq=%d,temperature=%.1f\n",data[i].seq,data[i].temp);

    }
    //写入完成，关闭写端
    printf("write finish\n");
    close(sensor_pipe[1]);
    printf(" close sensor_pipe write fd\n");
    return NULL;
}



int main(void)
{
    //创建一个pipe
    if(pipe(sensor_pipe)==-1)
    {
        perror("pipe");
        return 1;
    }

    //创建生产者线程
    pthread_t producer;
    int ret=pthread_create(&producer,NULL,producer_thread,NULL);
    if(ret!=0)
    {
        //创建线程失败，打印错误码
        fprintf(stderr,"producer create fail:%s\n",strerror(ret));
        //关闭pipe
        close(sensor_pipe[0]);
        close (sensor_pipe[1]);
        return 1;
    }

    //创建poll_fd数组
    struct pollfd fds[FD_COUNT]={
        {
            .fd=sensor_pipe[0],//读端
            .events=POLLIN,
            .revents=0,//初始化为0
        },

        {
            .fd=STDIN_FILENO,
            .events=POLLIN,
            .revents=0,
        }
    };

    int finish=0;
    printf("type status and press enter\n");

    while(!finish)
    {
        printf("[main] waiting for stdin or sensor...\n");
        int ready=poll(fds,FD_COUNT,-1);//阻塞等待fd事件
        if(ready==-1)
        {
            if(errno==EINTR)
            {
                continue;
            }

            perror("poll");
            break;
        }

        //等到事件：终端输入命令/有传感器数据可读

        //处理传感器数据
        if(fds[0].revents&POLLIN)
        {
            sensor_data_t data;
            ssize_t n=read(sensor_pipe[0],&data,sizeof(data));

            if(n==-1)
            {
                perror("sensor read");
                finish=1;
            }
            else if (n==0)
            {
                /* code */
                printf("sensor read EOF\n");
                finish=1;
                
            }
            else if (n!=sizeof(data))
            {
                fprintf(stderr,"sensor read incomplete byte:%zd/%zu\n",n,sizeof(data));
                finish=1;
            }
            else
            {
                printf("sensor read :seq=%d,temperature=%.1f\n",data.seq,data.temp);
            }
            
        }

        //处理打印命令
        if(fds[1].revents&POLLIN)
        {
            char cmd[32];
            ssize_t n=read(STDIN_FILENO,cmd,sizeof(cmd)-1);
            
            if(n==-1)
            {
                perror("[main] read");
                
            }
            else if (n==0)
            {
                /* code */
                printf("CMD read EOF\n");
            }
            else
            {
                cmd[n]='\0';
                printf("cmd:%s\n",cmd);
            }
        }

        //挂起且没有数据要读取
        if(!(fds[0].revents&POLLIN)&&(fds[0].revents&POLLHUP))
        {
            printf("[main] sensor pipe hang up\n");
            finish=1;
        }

        if(fds[0].revents&(POLLERR|POLLNVAL))
        {
            printf("[main] sensor pipe error or invalid\n");
            finish=1;
        }

       

    }

    //finish=1
    close(sensor_pipe[0]);



    //删除线程
    ret=pthread_join(producer,NULL);
    if(ret!=0)
    {
        fprintf(stderr,"pthread_join fail:%s\n",strerror(ret));
        return 1;
    }

    printf("[main] all work finished\n");

    return 0;
}