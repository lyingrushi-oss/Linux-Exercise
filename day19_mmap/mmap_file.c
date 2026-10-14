#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(void)
{

    int fd=open("sensor.txt",O_RDWR);
    if(fd==-1)
    {
        perror("open");
        return 1;
    }

    off_t file_size=lseek(fd,0,SEEK_END);
    if(file_size==-1)
    {
        perror("lseek");
        close(fd);
        return 1;
    }
    if(file_size==0)
    {
        fprintf(stderr,"sensor.txt is empty.\n");
        close(fd);
        return 1;

    }
    //成功获取文件大小

    //建立文件虚拟映射
    char* map=mmap(NULL,(size_t)file_size,
                    PROT_READ|PROT_WRITE,MAP_PRIVATE,fd,0);
    if(map==MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return 1;
    }

    printf("before:");
    fflush(stdout);//flush stream 刷新流式缓冲区
    write(STDOUT_FILENO,map,(size_t)file_size);

     /*
     * "temperature=" 共 12 字节，
     * 因此温度值从 map[12] 开始。
     */
    memcpy(map+12,"28.5",4);

    if(msync(map,(size_t)file_size,MS_SYNC)==-1)
    {
        perror("msync");
        //解除内存映射
        munmap(map,(size_t)file_size);
        close(fd);
        return 1;
    }


    printf("after:");
    fflush(stdout);
    write(STDOUT_FILENO,map,(size_t)file_size);

    if(munmap(map,(size_t)file_size)==-1)
    {
        perror("munmap");
        close(fd);
        return 1;
    }

    close(fd);

    return 0;
}