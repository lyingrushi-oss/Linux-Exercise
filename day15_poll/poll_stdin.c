#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

int main(void)
{

    //创建一个poll_fd结构体
    struct pollfd pfd={
        .fd=STDIN_FILENO,//fd
        .events=POLLIN,//用户关心的事件
        .revents=0,//内核实际发生返回的事件
    };

    //准备接收缓冲区
    char buf[64];

    while(1)
    {
        printf("waiting for STDIN_FILENO pollin EVENT\n");
        //poll这个函数借口是会阻塞等待的，如果把上面的printf放在下面就会导致5s以后才打印日志
        int ret=poll(&pfd,1,5000);//阻塞等待的fd个数：1个，超时时间5000ms
       
        if(ret==-1)
        {
            //错误返回
            if(errno==EINTR)
            {
                continue;
            }
            perror("poll");
            return 1;
        }

        //阻塞超时还未有事件发生
        if(ret==0)
        {
            printf("poll waiting timeout:no input\n");
            continue;//重新进入阻塞等待
        }

        //等到事件了
        if(pfd.revents&POLLIN)
        {
            printf("POLLIN EVENT happen!!\n");
            
            ssize_t n=read(pfd.fd,buf,sizeof(buf)-1);
            if(n==-1)
            {
                perror("read");
                return 1;
            }
            //读取到EOF
            if(n==0)
            {
                printf("read EOF\n");
                break;
            }
            //读取到数据
            buf[n]='\0';
            printf("read %zd bytes:%s\n",n,buf);
        }
        //发生错误了
        if(pfd.revents&(POLLERR|POLLHUP))
        {
            //错误或者挂起
            printf("stdin error or hangup\n");
            break;//读取结束或者错误/挂起退出循环
        }
        
    }
    return 0;
    
}