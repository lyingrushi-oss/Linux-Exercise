#include <stdio.h>
#include "sensor.h"

int main(void)
{
    for(int i=0;i<3;i++)
    {
        float temp=read_temp(i);
        printf("seq=%d,temp=%.1f\n",i,temp);
    }
    printf("修改main.c\n");
    return 0;
}