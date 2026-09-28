#include "sensor.h"

float read_temp(int seq)
{
    return 26.0f+0.5f*seq;
}