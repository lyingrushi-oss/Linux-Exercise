#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
    const char msg[]="26.0\n";
    char buf[5];
    //打开文件
    int fd =open("temp.txt",O_WRONLY|O_TRUNC|O_CREAT,0644);
    if(fd==-1)
    {
        //打开失败了
        perror(" write open fail");
        return 1;
    }
    printf("write open success!!!\n");
    ssize_t ret=write(fd,msg,(sizeof(msg)-1));
    if(ret!=(sizeof(msg)-1))
    {
        if(ret==-1)
        {
            //写入失败
            perror("write");
        }
        else
        {
            //不完整写入
            printf("incomplete write:%zd\n",ret);
        }
        close(fd);
        return 1;
    }
    //成功写入了，可以读取了
    fd=open("temp.txt",O_RDONLY);
    if(ret==-1)
    {
        perror("read open fail!!");
        return 1;
    }
    ssize_t n=0;
    #if 1
    while((n=read(fd,buf,4))>0)
    {
        buf[n]='\0';
        printf("read bytes:%zd\n",n);
        printf("[recv]%s\n",buf);
    }
    #endif
    
    
    #if 0
    //短读代码
    size_t count=0;
    while(count<16)
    {

        //第二个参数，本次读取要存放位置的首地址
        //第三个参数，本次期望读取到的字节数
    
        n=read(fd,buf+count,16-count);//这里的指针和size记得做count偏移
        if(n>0)
        {
            count+=n;

        }
        else if(n=0)
        {
           fprintf(stderr,"only read %zu bytes EOF\n",count);
            break;
        }
        else
        {
            perror("read");
            close(fd);
            return 1;
        }

    }
        #endif
    if(n==-1)
    {
        perror("read");
        close(fd);
        return 1;
    }
    printf("read EOF!!!\n");
    close(fd);
    return 0;


}