#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

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

#if 0
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
#endif

/// @brief 从文本中解析出温度
/// @text 文本指针
/// @param temp_out 输出温度值
/// @return 成功/失败
static int parase_temp(const char* text,float * temp_out)
{
    char* end=NULL;

    errno=0;//对错误原因清零
    float temp=strtof(text,&end);

    //没有解析到温度数值
    if(end==text)
    {
        fprintf(stderr,
        "[ERROR] invalid temperature:no number\n");
        return -1;
    }

    //超出浮点范围
    if(errno==ERANGE)
    {
        fprintf(stderr,
        "[ERROR] invalid temperature:out of float range\n");
        return -1;
    }

    //跳过空格
    while(isspace((unsigned char)*end)!='\0')
    {
        end++;
    }

    if(*end!='\0')
    {
        fprintf(stderr,
        "[ERROR] invalid temperature:unexpected character\"%c\"\n",*end);
        return -1;
    }

    if(!isfinite(temp))
    {
        fprintf(stderr,
        "[ERROR] invalid temperature:no finite\n");
        return -1;
    }
    
    if(temp<-40.0f||temp>125.0f)
    {
        fprintf(stderr,
        "[ERROR] invalid temperature:out of sensor range\n");
        return -1;
    }

    *temp_out=temp;
    return 0;

}


//day26读取传感器文件接口
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

   // 2.读取传感器文件
   size_t used=0;
   while(used<capacity-1)
   {
        //一次读取，，read不保证一次读完
        ssize_t n=read(fd,buf+used,capacity-1-used);
        //读取失败
        if(n==-1)
        {
            log_error("sensor file read failed",path);
            close(fd);
            return -1;
        }
        if(n==0)
        {
            break;
        }

        used+=(size_t)n;
        
   }

   // 3.检查文件是否超长
   if(used==capacity-1)
   {
        //额外多读一个字节
        char extra;
        ssize_t n=read(fd,&extra,1);
        //读取失败
        if(n==-1)
        {
            log_error("sensor file read fail",path);
            close(fd);
            return -1;
        }
        
        if(n>0)
        {
            //说明超过31个字节
            fprintf(stderr,"[ERROR] sensor file too long :path=%s,max=%zu bytes\n",path,capacity-1);
            close(fd);
            return -1;
        }


        
   }

   //4.正常返回<=31个字节和文件为空处理
   if(used==0)
   {
        log_error("sensor file is empty",path);
        close(fd);
        return -1;
   }

   buf[capacity-1]='\0';
   if(close(fd)==-1)
   {
        log_error("sensor file close  failed",path);
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

    float temp;
    if(parase_temp(buf,&temp)!=0)
    {
        fprintf(stderr,
        "[ERROR] sensor data validation failed\n");
        return EXIT_FAILURE;
    }

    printf("[INFO] 温度=%.1f\n",temp);
    return EXIT_SUCCESS;
}