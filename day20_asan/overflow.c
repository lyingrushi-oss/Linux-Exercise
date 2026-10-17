#include <stdio.h>

static void collect_sample(void)
{
    int sample[4]={25,26,27,28};
    //防止越界：最好不要手写数组大小，使用sizeof关键字，并注意最好不要使用sizeof(类型)
    size_t count=sizeof(sample)/sizeof(sample[0]);
    for(size_t i=0;i<count;i++)
    {
        printf("sample[%zu]=%d\n",i,sample[i]);
    }
}

int main(void)
{
    collect_sample();
    return 0;
}