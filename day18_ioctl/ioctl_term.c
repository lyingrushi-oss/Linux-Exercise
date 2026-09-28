#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(void)
{
    //声明一个结构体变量
    struct winsize size;
    int ret=ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
    if(ret==-1)
    {
        fprintf(stderr,"ioctl TIOCGWINSZ :%s\n",strerror(errno));
        return 1;
    }

    printf("terminal windows rows=%u,columns=%u\n",(unsigned int)size.ws_row,
    (unsigned int)size.ws_col);


    return 0;
}