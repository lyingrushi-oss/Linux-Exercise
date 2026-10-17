#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

//定义结构体
typedef struct {
    int * payload;//一个指向int的指针
}packet_t;

int main(void)
{
    //1.申请一块内存存放packet_t类型的变量，保存指针
    packet_t* packet=malloc(sizeof(*packet));

    //2.检查是否申请成功，失败退出
    if(packet==NULL)
    {
        fprintf(stderr,"packet malloc failed\n");
        return 1;
    }

    //3.申请一块内存，用于存放100个整型变量，把指针存放到payload里面
    packet->payload=malloc(100*sizeof(int));

    //4.申请失败退出
    if(packet->payload==NULL)
    {
        fprintf(stderr,"payload malloc failed\n");
        //释放掉packet指针
        return 1;
    }

    packet->payload[0] = 5;
    printf("第一个数据=%d\n", packet->payload[0]);

    /* 故意不释放，观察直接泄漏与间接泄漏 */
    //先释放子对象
    free(packet->payload);
    //再释放指向子对象的父对象
    free(packet);//二级指针，指向指针的指针



    return 0;
}