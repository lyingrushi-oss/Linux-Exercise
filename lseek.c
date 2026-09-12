#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
int fd= open("test.txt",O_RDONLY);
char buf[4]={0};
lseek(fd,5,SEEK_SET);
read(fd,buf,3);
printf("buf :%s\n",buf);
char buf1[5]={0};
char buf2[5]={0};
lseek(fd,2,SEEK_CUR);
read(fd,buf1,3);
printf("buf1 :%s\n",buf1);
lseek(fd,-6,SEEK_END);
read(fd,buf2,5);
printf("buf2 :%s\n",buf2);
close(fd); 
return 0;

}
