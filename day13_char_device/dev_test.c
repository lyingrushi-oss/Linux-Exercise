#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{

    int null_fd=open("/dev/null",O_WRONLY);
    if(null_fd==-1)
    {
        perror("open null fd");
        return 1;
    }
    ssize_t count=write(null_fd,"ABC",3);
    if(count==-1)
    {
        perror("null write");
        return 1;
    }
    else
    {
        if(count!=3){
             printf("/dev/null incomplete write:%zd bytes\n",count);
       return 1;
        }
        else
        {
            printf("write success\n");
        }
      
    }
    char buf[4];
    int zero_fd=open("/dev/zero",O_RDONLY);
    if(zero_fd==-1)
    {
        perror("open /dev/zero");
        close(null_fd);
        return 1;
    }
    count=read(zero_fd,buf,sizeof(buf));
    if(count==-1)
    {
        perror("read /dev/zero");
        close(null_fd);
        return 1;
    }
    printf("read %zd bytes \n",count);
    for (ssize_t i = 0; i < count; i++) {
        printf("%02x ", (unsigned char)buf[i]);
    }
    close(null_fd);
    close(zero_fd);


    return 0;
}