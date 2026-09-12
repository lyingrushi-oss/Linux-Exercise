/*
    父子进程两个管道双向通信：父进程发送指令子进程响应

*/
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

/*请求和响应消息结构体定义*/
typedef struct{
    int cmd;
    int value;
}request_t;

typedef struct{
    int status;
    int result;
}respond_t;


int main(void)
{
    //#1.创建两个管道ptc和ctp
    int ptc[2];
    int ctp[2];
    int p1=pipe(ptc);
    if(p1==-1)
    {
        //创建失败
        perror("ptc pipe");
        return 1;
    }
    int p2=pipe(ctp);
    if(p2==-1)
    {
        //创建失败
        perror("ctp pipe");
        return 1;
    }
    //#2.创建两个进程
    pid_t pid=fork();
    if(pid==-1)
    {
        //fork失败,释放pipe
        perror("fork");
        close(ptc[0]);
        close(ptc[1]);
        close(ctp[0]);
        close(ctp[1]);
        return 1;
    }
    //创建成功，子进程分支
    if(pid==0)
    {
        printf("[child] pid=%d\n",(pid_t)(getpid()));
        //1.ptc作为接收端，关闭写端
        close(ptc[1]);
        //2.ctp作为写端，关闭接收
        close(ctp[0]);
        //3.等待父进程的命令
        request_t req;
        respond_t resp;
        ssize_t recv=read(ptc[0],&req,sizeof(req));
        //4.处理接收结果
        //4.1接收失败
        if(recv==-1)
        {
            perror("[child] read");
            //关闭读端和写端
            close(ptc[0]);
            close(ctp[1]);
            return 1;
        }
        //4.2没接收完整
        if(recv!=(ssize_t)(sizeof(req)))
        {
            fprintf(stderr,"[child] incomplete recv:%zd/%zu\n",recv,sizeof(req));
            close(ptc[0]);
            close(ctp[1]);
            return 1;
        }
        //4.3完整接收
        //打印接收结果
        printf("[child]recv:cmd=%d,value=%d\n",req.cmd,req.value);

        if(req.cmd==1)
        {
            resp.status=0;
            resp.result=req.value*2;
        }
        else
        {
            resp.status=-1;
            resp.result=0;
        }

        //准备发送响应
            ssize_t wrt=write(ctp[1],&resp,sizeof(resp));
            if(wrt==-1)
            {
                perror("[child] write");
                close(ptc[0]);
                close(ctp[1]);
                return 1;
            }
            if(wrt!=(ssize_t)(sizeof(resp)))
            {
                fprintf(stderr,"[child] incomplete send:%zd/%zu\n",wrt,sizeof(resp));
                close(ptc[0]);
                close(ctp[1]);
                return 1;
            }
            //成功发送
            printf("[child] sent,status=%d,result=%d\n",resp.status,resp.result);
            close(ptc[0]);
            close(ctp[1]);
            return 0;
    }
    //父进程
    printf("[parent]:child pid=%d\n",pid);
    close(ptc[0]);
    close(ctp[1]);
    request_t request;
    respond_t respond;
    request.cmd=99;
    request.value=2;
    ssize_t sent=write(ptc[1],&request,sizeof(request));
    if(sent==-1)
    {
        perror("[parent] write");
        close(ptc[1]);
        close(ctp[0]);
        return 1;
    }
    if(sent!=(ssize_t)(sizeof(request)))
    {
        fprintf(stderr,"[parent] incompelte sent:%zd/%zu\n",sent,sizeof(request));
        //关掉写端
        close(ptc[1]);
        close(ctp[0]);
        return 1;
    }
    //发送成功
    close(ptc[1]);
    printf("parent sent:cmd=%d,value=%d\n",request.cmd,request.value);
    ssize_t get=read(ctp[0],&respond,sizeof(respond));
    if(get!=(ssize_t)(sizeof(respond)))
    {
        if(get==-1)
        {
            perror("[parent] read");
            close(ctp[0]);
            return 1;
        }
        else{
            fprintf(stderr,"[parent] incompelte recv:%zd/%zu\n",get,sizeof(request));
            close(ctp[0]);
            return 1;
        }
    }
    //接收成功
    printf("parent recv:status=%d,result=%d\n",respond.status,respond.result);
    close(ctp[0]);
    //等待子进程返回
    int status;
    pid_t id=waitpid(pid,&status,0);
    if(id==-1)
    {
        perror("waitpid");
        return 1;
    }
    if(WIFEXITED(status))//是否正常退出的宏
    {
        //打印退出码的宏WEXITSTATUS
        printf("[parent] child exit code:%d\n",WEXITSTATUS(status));
        
    }
    return 0;

}