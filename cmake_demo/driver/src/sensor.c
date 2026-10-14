#include "sensor.h"
#include "crc.h"

int sensor_read(void)
{
    return 25;
}

int sensor_read_crc(void)
{
    int value = sensor_read();

    return crc_calculate(value);
}