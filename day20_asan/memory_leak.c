#include <stdio.h>
#include <stdlib.h>


int main(void)
{

    //申请一个int数组，100个元素
    int *sample=malloc(100*sizeof(*sample));

    //申请失败
    if(sample==NULL)
    {
        fprintf(stderr,"申请失败！！\n");
        return 1;
    }

    //赋值
    sample[0]=5;
    printf("第一个sample=%d\n",sample[0]);


    //故意不释放内存，让内存泄漏
    //free(sample);
    return 0;
}