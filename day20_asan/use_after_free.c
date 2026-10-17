#include <stdio.h>
#include <stdlib.h>

typedef struct{
    int seq;
    float temp;
}sensor_data_t;


int main(void)
{

    //申请一块内存
    sensor_data_t* data=malloc(sizeof(sensor_data_t));
    if(data==NULL)
    {
        fprintf(stderr,"malloc failed,申请内存失败！！！\n");
        return 1;
    }

    //向内存中写值
    data->seq=1;
    data->temp=26.7f;

    printf("seq=%d,temperature=%.1f\n",data->seq,data->temp);
    
    //释放掉内存
    free(data);//只是释放这块内存，但是这个指针变量仍然保存旧的地址值，成为悬空指针

    //显示修改指针变量的值为NULL
    data=NULL;

    //

    

    return 0;
}