#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>

/*定义传感器数据结构体*/
typedef struct{
int seq;//序列号
float temp;//温度
}sensor_data_t;

int main()
{
    int fd[2];
    int p_ret=pipe(fd);
    if(p_ret==-1)
    {
        //pipe创建失败
    perror("pipe");
    return 1;
    }
    //创建复制进程
    pid_t pid= fork();
    //判断是否创建成功，失败提前返回

    if(pid==-1)
    {
        //fork失败，提前返回，关闭fd[2]
    perror("fork");
    close(fd[0]);
    close(fd[1]);
    return 1;
    }

    //判断父子进程
    if(pid==0)
    {
    //子进程：只读不写
    close(fd[1]);
    sensor_data_t data;
    ssize_t bytes_read;
    printf("[child]PID=%ld,waiting for data...\n",(long)getpid());
    while((bytes_read=read(fd[0],&data,sizeof(data)))>0)
    {
    //长度不匹配
    if(bytes_read!=sizeof(data))
    {
    fprintf(stderr,"[child] incomplete length:%zu\n",bytes_read);
    close(fd[0]);
    return 1;
    }
    //接收成功
    printf("[child]:sequence=%d,temperature=%.1f C\n",data.seq,data.temp);
    }/*while循环结束：返回值>0,下面是0和-1*/
    //读取返回-1
    if(bytes_read==-1)
    {
    perror("[child] read");
    close(fd[0]);
    return 1;
    }

    printf("read return 0:all write close end\n");
    close(fd[0]);
    return 0;
    }
    //父进程
    close(fd[0]);
    printf("[parent]pid=%ld,[child]pid=%ld\n",(long)getpid(),(long)pid);
    for (int i = 0; i < 3; i++) {
            sensor_data_t data = {
                .seq = i + 1,
                .temp = 25.0f + i * 0.5f
            };
    ssize_t written = write(fd[1],&data,sizeof(data));//修改一次向pipe写一次
    /*返回值情况分类：-1 >0*/
    if(written==-1)
    {
        //写失败
    perror("[parent] write");
    close(fd[1]);
    waitpid(pid,NULL,0);
    return 1;
    }
    //写的序列号
    printf("[parent] sent sequence=%d\n", data.seq);
            sleep(1);
    }/*注意改了三次写了三次，此时for循环才结束*/

    printf("[parent]write end,close write\n");
    close(fd[1]);

    int status;
    waitpid(pid,&status,0);
    printf("[parent]child exit code:%d\n",WEXITSTATUS(status));

}
