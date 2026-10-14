#include <stdio.h>
#include "sensor.h"

int main(void)
{
    int value = sensor_read();
    int crc = sensor_read_crc();

    printf("sensor value = %d, crc = %d\n", value, crc);

    return 0;
}