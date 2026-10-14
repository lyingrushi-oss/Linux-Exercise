#include <stdio.h>

//定义数据包结构体
typedef struct{

    float temp;
    int seq;
}sensor_packet_t;

static float read_temp(const sensor_packet_t* pack)
{
    return pack->temp;
}

static int process_packet(const sensor_packet_t* pack)
{

    //1️⃣加防御性编程：空指针
    if(pack==NULL) 
    {
        fprintf(stderr,"[error] packet is NULL!!\n");
        return -1;
    }
    float temp=read_temp(pack);
    printf("温度=%.1f\n",temp);
    //2️⃣增加返回值，上报main，区分正常和错误
    return 0;
}

int main(void)
{

    // sensor_packet_t* pack=NULL;
    // int ret=process_packet(pack);

    
    sensor_packet_t data={
        .temp=25.0,
    };
    int ret=process_packet(&data);

    if(ret==-1) return 1;//错误上报main,异常返回，用非零退出码上报失败


    return 0;
}