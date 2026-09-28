#include <stdio.h>
#include <string.h>
#include <sys/types.h>


typedef ssize_t (*read_fn_t)(char* buf,size_t len);

/*模拟内核中的操作表*/
typedef struct {
    const char* name;
    read_fn_t read;
}fake_file_t;

/*声明一个传感器读取的函数*/
static ssize_t sensor_read(char* buf,size_t len)
{
    char temp[]="temp=25.0\n";
    size_t data_len=sizeof(temp)-1;
    if(data_len>len)
    {
        data_len=len;//不可以超过读取缓冲区的大小
    }
    memcpy(buf,temp,data_len);
    return data_len;//返回读取的长度
}

/*声明一个类/dev/null设备属性的函数*/
static ssize_t null_read(char* buf,size_t len)
{
    (void)buf;
    (void)len;
    printf("数据黑洞\n");
    return 0;//数据黑洞
}

/*声明一个读取分发函数*/
static ssize_t vfs_read_dispatcher(fake_file_t* file,char* buf,size_t len)
{
    if(file==NULL||buf==NULL)
    {
        return -1;
    }
    printf("vfs dispatcher to:%s\n",file->name);
    return file->read(buf,len);
}
int main(void)
{
    fake_file_t sensor={
        .name="sensor",
        .read=sensor_read,
    };

    fake_file_t dev_null={
        .name="dev_null",
        .read=null_read,
    };

    char buf[32];
    ssize_t n;

    n=vfs_read_dispatcher(&sensor,buf,sizeof(buf)-1);
    
    if(n>0)
    {
        buf[n]='\0';
        printf("sensor read %zd bytes,%s\n",n,buf);
    }

    n=vfs_read_dispatcher(&dev_null,buf,sizeof(buf)-1);

    if(n==0)
    {
        printf("null read: 0 bytes (EOF)\n");
    }

    return 0;
}