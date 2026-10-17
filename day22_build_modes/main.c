#include <stdio.h>
#include "sensor.h"


int main(void)
{
    //准备一个温度数组
    float sample[]={24.5,23.4,25.5,26.7};
    int count=(int)(sizeof(sample)/sizeof(sample[0]));

    float average=sensor_average(sample,count);
    printf("平均温度是%.1f\n",average);


    return 0;
}