#include "sensor.h"

float sensor_average(const float* sample,int count)
{
    float sum=0.0f;

    
    for(int i=0;i<count;i++)
    {
        sum+=sample[i];
    }

    return sum /count;
}