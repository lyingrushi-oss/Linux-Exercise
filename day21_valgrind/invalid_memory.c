#include <stdio.h>
#include <stdlib.h>

int main(void)
{

    //申请一块内存
    int* data=malloc(10*sizeof(*data));

    if(data==NULL)
    {
        return 1;
    }

    data[0]=25;
    free(data);//释放内存

    /*第一个错误:*/
    //非法访问指向被释放的内存的指针
    printf("data[0]=%d\n",data[0]);

    /*第二个错误：重复释放*/
    free(data);



    return 0;
}