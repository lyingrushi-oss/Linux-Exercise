#include <stdio.h>
#include <stdlib.h>

int main(void)
{

    int* data=malloc(sizeof(*data));

    if(data==NULL)
    {
        return 1;
    }

    /*给data赋初值*/
    *data=0;

    //只分配内存不初始化赋初值，用它做条件判断
    if(*data>0)
    {
        printf("是正数\n");

    }


    free(data);
    return 0;
}