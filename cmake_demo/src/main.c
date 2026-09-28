#include <stdio.h>
#include "sensor.h"

int main(void)
{
    int value = sensor_read();

    printf("sensor value = %d\n", value);

    return 0;
}