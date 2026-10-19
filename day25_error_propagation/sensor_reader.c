#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

/*  
    区分系统错误和业务错误
*/

static void log_error(const char* operation,const char* path)
{
    //保存当前错误编号errno
    int saved_error=errno;
    fprintf(stderr,
    "[ERROR] %s:path=%s errno=%d(%s)\n",
    operation,path,saved_error,strerror(saved_error));
}

#if 0
static int check_sensor_file(const char* path)
{
    //打开文件
    int fd=open(path,O_RDONLY);
    if(fd==-1)
    {
        log_error("sensor file open failed",
        path);
        return -1;
    }
    close(fd);
    return 0;
}
#endif

//读取传感器文件接口
static int read_sensor_file(const char* path,char* buf,size_t capacity)
{
    // 1.根据传入路径打开传感器文件
    int fd=open(path,O_RDONLY);
    if(fd==-1)
    {
        //打开失败退出
        log_error("sensor file open failed!",path);
        return -1;

    }

    // 2.成功打开，读取数据
    ssize_t n=read(fd,buf,capacity-1);

    // 3.检查返回值
    if(n==-1)
    {
        log_error("sensor file read failed", path);
        return -1;
    }

    if(n==0)
    {
        fprintf(stderr,"sensor file is empty,path=%s\n",path);
        return -1;
    }

    // 4.成功读取到值
    buf[n]='\0';

    // 5.读取完成，关闭文件
    if(close(fd)==-1)
    {
        log_error("sensor file close failed",path);
        return -1;
    }
    return 0;
}


/*
    程序流：
    Shell传入文件路径（通过main函数的参数由shell传入，命令./sensor_reader sensor.txt）
    其中./sensor_reader为第一个参数对应argv[0],sensor.txt作为第二个参数argv[1]
    → main检查参数
    → main准备接收缓冲区
    → read_sensor_file打开文件
    → 读取文件内容
    → 补上字符串结束符'\0'
    → 关闭文件
    → 将成功或失败返回main
    → main决定进程退出码
*/
//argc是传入参数数量，argv是参数字符串列表argument vector
int main(int argc,char* argv[])
{
    if(argc!=2)
    {
        fprintf(stderr,"usage: %s <sensor file>\n",argv[0]);
        return EXIT_FAILURE;
    }

    #if 0
    if(check_sensor_file(argv[1])==-1)
    {
        fprintf(stderr,"[ERROR] sensor initialization failed\n");
        return EXIT_FAILURE;
    }
    #endif

    char buf[32];
    if(read_sensor_file(argv[1],buf,sizeof(buf))==-1)
    {
        fprintf(stderr,"[ERROR] sensor initialization failed!\n");
        return EXIT_FAILURE;
    }

    printf("[INFO] sensor file is ready!\n");
    printf("[INFO] sensor value=%s",buf);
    return EXIT_SUCCESS;
}