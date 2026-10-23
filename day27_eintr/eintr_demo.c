/*
    父进程创建管道并fork
            ↓
    子进程等待2秒
            ↓
    父进程设置1秒后的SIGALRM
            ↓
    父进程阻塞在read
            ↓
    第1秒：SIGALRM到达，read返回-1/EINTR
            ↓
    父进程识别EINTR并重试read（使用action.sa_flags=SA_RESTRART不暴露eintr给用户代码
            ↓
    第2秒：子进程写入"25.0\n"
            ↓
    父进程read成功
*/


#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//在这里充当一个标志位，代表有闹钟信号发生过
static volatile sig_atomic_t alarm_seen=0;//C语言原子整形

static void handle_alarm(int signo)
{
    (void) signo;
    alarm_seen=1;
}



int main(void)
{
    //创建pipe
    int fd[2];
    int ret=pipe(fd);
    if(ret==-1)
    {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t pid=fork();//创建子进程

    if(pid==-1)
    {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return EXIT_FAILURE;
    }

    if(pid==0)
    {
        //子进程
        //子进程只写
        close(fd[0]);
        //睡两秒
        sleep(2);
        const char temp[]="25.9";
        if(write(fd[1],temp,sizeof(temp)-1)==-1)
        {
            perror("write");
            close(fd[1]);
            _exit(EXIT_FAILURE);
        }
        //成功写入
        _exit(EXIT_SUCCESS);
    }
    //父进程：只读
    close(fd[1]);
    /*
    struct sigaction {
    //这两个函数指针是共用一块存储的联合体，只能二选一
    void     (*sa_handler)(int);   // 简单处理函数，其中参数sig是要处理的信号的编号
    void     (*sa_sigaction)(int, siginfo_t *, void *); // 带信息的处理函数
    sigset_t   sa_mask;            // 处理函数执行期间额外屏蔽的信号集
    int        sa_flags;           // 行为标志
    void     (*sa_restorer)(void); // 已废弃/内部用
    };
    */
    struct sigaction action={0};//信号处理行为结构体

    action.sa_handler=handle_alarm;//信号到来时要执行的函数
    sigemptyset(&action.sa_mask);//初始化信号集为空

    //action.sa_flags=0;
    action.sa_flags=SA_RESTART;//read系统调用自动恢复，不上报给用户代码

    //int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
    //真正执行信号行为注册的函数
    if(sigaction(SIGALRM,&action,NULL)==-1)
    {
        perror("sigaction");
        close(fd[0]);
        waitpid(pid,NULL,0);
        return EXIT_FAILURE;
    }

    alarm(1);//1s后向当前进程发送一次SIGALRM
    char buf[32];
    size_t used=0;

    for(;;)
    {
        ssize_t n=read(fd[0],buf+used,sizeof(buf)-1-used);
        if(n>0)
        {
            //读取成功
            used+=(size_t)n;
            continue;
        }
        if(n==0)
        {
            //EOF
            //放循环外面buf[n]='\0';
            break;
        }
        
        if(errno==EINTR)
        {
            int saved_error=errno;
            printf("[parent] read interrupt:errno=%d(%s),alarm_seen=%d\n",saved_error,
                strerror(saved_error),alarm_seen
            );
            continue;
        }

        //读取出错
        perror("read");
        close(fd[0]);
        waitpid(pid,NULL,0);
        return  EXIT_FAILURE;
    }

    //正常读取完成后回收资源退出
    buf[used]='\0';//记得补\0
    close(fd[0]);
    waitpid(pid,NULL,0);
    printf("[parent] read success: %s\n",buf);
    printf("[parent] alarm_seen=%d\n", alarm_seen);
    return EXIT_SUCCESS;






    return 0;
}